#include <diploma.hpp>

#define SECONDS_IN_DAY 86400

void diploma::on_asset_transfer(name from, name to, const vector<uint64_t> &asset_ids, const string &memo) {
    if (from == _self) {
        return;
    }

    if (to != _self) {
        transfer_handler(from, to, asset_ids, memo);
        return;
    }

    auto player_pets = get_player_pets(from);
    auto player_info_t = get_player_info(from);
    auto player_info = player_info_t.get_or_create(_self);

    for (auto asset_id: asset_ids) {
        auto asset = get_asset(to, asset_id);

        if (asset.asset.schema_name == FOODS) {
            player_info.foods.push_back(asset_id);
            continue;
        }

        check(asset.asset.schema_name == ANIMALS, "Asset " + to_string(asset_id) + " is not an animal");

        auto pet = player_pets.find(asset_id);

        check(pet != player_pets.end(), "Pet doesn't belong to you");

        player_pets.modify(pet, _self, [&](player_pets_record &rec) {
            rec.last_walk_start_ts = current_time_point().sec_since_epoch();
            rec.status = ON_A_WALK;
        });
    }

    player_info_t.set(player_info, _self);
}

void diploma::feed(name player, uint64_t pet_asset_id, uint64_t food_asset_id) {
    require_auth(player);

    auto player_pets = get_player_pets(player);
    auto player_info_t = get_player_info(player);
    auto player_info = player_info_t.get_or_default();

    auto pet = animal_repository.read_animal(player, pet_asset_id);
    auto food = food_repository.read_food(_self, food_asset_id);
    auto food_itr = std::find(player_info.foods.begin(), player_info.foods.end(), food_asset_id);

    check(food_itr != player_info.foods.end(), "Can't feed with this food: " + to_string(food_asset_id));

    auto pet_itr = player_pets.find(pet_asset_id);
    uint32_t curr_ts = current_time_point().sec_since_epoch();

    check(pet.is_alive(), "Your pet is dead");

    auto pet_attributes = pet.get_attributes();

    if (pet_itr == player_pets.end()) {
        player_pets.emplace(_self, [&](player_pets_record &rec) {
            rec.asset_id = pet_asset_id;
            rec.last_feed_ts = curr_ts;
        });

    } else {
        if ((pet_itr->last_feed_ts - curr_ts) > (SECONDS_IN_DAY * 3)) {
            pet_attributes.satisfaction_points = 0;
            animal_repository.write_animal(player, pet);

            aa::transfer_asset(_self, player, {food_asset_id}, "Can't feed a dead pet, I'm sorry...");

            return;
        }

        player_pets.modify(pet_itr, _self, [&](player_pets_record &rec) {
            rec.last_feed_ts = curr_ts;
        });
    }

    set_satisfaction_points(pet, 0, food.get_healthiness());

    aa::burn_asset(_self, food_asset_id);
    player_info.foods.erase(food_itr);

    animal_repository.write_animal(player, pet);
}

void diploma::on_burn_asset(name owner, uint64_t asset_id, name collection_name, name schema) {
    if (collection_name != COLLECTION) {
        return;
    }

    if (schema != FOODS) {
        return;
    }

}

void diploma::claim_balance(name player) {
    require_auth(player);

    auto player_info_t = get_player_info(player);
    auto player_info = player_info_t.get_or_default();

    eosio::token::transfer_action action(TOKEN_ACCOUNT, {get_self(), "active"_n});
    action.send(get_self(), player, player_info.balance, "Claimed charms");

    player_info.balance.amount = 0;

    player_info_t.set(player_info, _self);
}

void diploma::on_mint_asset(uint64_t asset_id, name authorized_minter, name collection_name, name schema_name,
                            int32_t template_id, name new_asset_owner) {
    if (collection_name != COLLECTION) {
        return;
    }

    if (schema_name != ANIMALS) {
        return;
    }

    auto player_pets = get_player_pets(new_asset_owner);

    auto iter = player_pets.find(asset_id);
    check(iter != player_pets.end(), "Something went wrong");

    player_pets.emplace(_self, [&](player_pets_record &rec) {
        rec.asset_id = asset_id;
    });
}

void diploma::set_satisfaction_points(Animal &animal, uint32_t walk_duration, uint64_t food_healthiness) {
    auto &attributes = animal.get_attributes();
    int rarity = animal.get_rarity();

    attributes.satisfaction_points += food_healthiness + (((walk_duration / 60) * rarity) % 50);
}

void diploma::lets_go_home(name player, uint64_t asset_id) {
    require_auth(player);

    auto player_pets = get_player_pets(player);
    auto player_info_t = get_player_info(player);
    auto player_info = player_info_t.get_or_default();

    auto pet = animal_repository.read_animal(_self, asset_id);

    auto pet_itr = player_pets.find(asset_id);
    check(pet_itr != player_pets.end(), "It's not your pet");
    check(pet_itr->status == ON_A_WALK, "Pet at home");

    auto curr_ts = current_time_point().sec_since_epoch();
    auto walk_duration = pet_itr->last_walk_start_ts - curr_ts;

    set_satisfaction_points(pet, walk_duration);

    animal_repository.write_animal(_self, pet);

    asset reward = calculate_walking_reward(pet.get_rarity(), walk_duration);

    player_pets.modify(pet_itr, _self, [&](player_pets_record &rec) {
        rec.last_walk_end_ts = curr_ts;
        rec.walking_reward = reward;
        rec.status = AT_HOME;
    });

    player_info.balance += reward;
    player_info_t.set(player_info, _self);

    aa::transfer_asset(_self, player, {asset_id}, "Claimed");
}

asset diploma::calculate_walking_reward(int rarity, uint32_t duration) {
    return asset(rarity * (duration / 60), TOKEN_SYMBOL);
}

void diploma::transfer_handler(name from, name to, const vector<uint64_t> &asset_ids, const string &memo) {
    auto from_pets = get_player_pets(from);
    auto to_pets = get_player_pets(to);

    for (auto asset_id: asset_ids) {
        auto from_pet_itr = from_pets.find(asset_id);
        check(from_pet_itr != from_pets.end(), "It's not your pet: " + to_string(asset_id));

        auto to_pet_itr = to_pets.find(asset_id);
        check(to_pet_itr == to_pets.end(), "Ohh.. wtf!?");

        to_pets.emplace(_self, [&](player_pets_record &rec) {
            rec.asset_id = asset_id;
        });

        from_pets.erase(from_pet_itr);
    }
}

