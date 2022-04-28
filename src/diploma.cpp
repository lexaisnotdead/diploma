#include <diploma.hpp>

#define SECONDS_IN_DAY 86400

void diploma::on_asset_transfer(name from, name to, const vector<uint64_t> &asset_ids, const string &memo) {
    if (to != _self) {
        return;
    }

    if (from == _self) {
        return;
    }

    auto player_pets = get_player_pets(from);

    for (auto asset_id: asset_ids) {
        auto asset = get_asset(to, asset_id);
        check(asset.asset.schema_name == ANIMALS, "Asset " + to_string(asset_id) + " is not an animal");

        auto pet = player_pets.find(asset_id);
        if (pet == player_pets.end()) {
            player_pets.emplace(_self, [&](player_pets_record &rec) {
                rec.asset_id = asset_id;
                rec.beginning_of_a_walk = current_time_point().sec_since_epoch();
            });
        } else {
            player_pets.modify(pet, _self, [&](player_pets_record &rec) {
                rec.beginning_of_a_walk = current_time_point().sec_since_epoch();
            });
        }
    }
}

void diploma::feed(name player, uint64_t pet_asset_id, uint64_t food_asset_id) {
    require_auth(player);

    auto player_pets = get_player_pets(player);

    auto pet = animal_repository.read_animal(player, pet_asset_id);
    auto food = food_repository.read_food(_self, food_asset_id);

    auto pet_itr = player_pets.find(pet_asset_id);
    uint32_t curr_ts = current_time_point().sec_since_epoch();

    check(pet.is_alive(), "Your pet is dead");

    auto pet_attributes = pet.get_attributes();

    if (pet_itr == player_pets.end()) {
        player_pets.emplace(_self, [&](player_pets_record &rec) {
            rec.asset_id = pet_asset_id;
            rec.last_feed_ts = curr_ts;
        });

        pet_attributes.satisfaction_points += food.get_healthiness();

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

        pet_attributes.satisfaction_points += food.get_healthiness();
    }

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

