#include <diploma.hpp>
#include <randomizer.hpp>

#define SECONDS_IN_DAY 86400

void diploma::on_asset_transfer(name from, name to, const vector<uint64_t> &asset_ids, const string &memo) {
    if (from == _self) {
        return;
    }

    if (to != _self) {
        transfer_handler(from, to, asset_ids, memo);
        return;
    }

    if (memo == "Craft") {
        craft(from, asset_ids);
        return;
    }

    if (memo == "Birth") {
        oh_my_godd__a_child(from, asset_ids);
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

void diploma::claim_balance(name player) {
    require_auth(player);

    auto player_info_t = get_player_info(player);
    auto player_info = player_info_t.get_or_default();

    eosio::token::transfer_action action(TOKEN_ACCOUNT, {get_self(), "active"_n});
    action.send(get_self(), player, player_info.balance, "Claimed charms");

    player_info.balance.amount = 0;

    player_info_t.set(player_info, _self);
}

void diploma::on_mint_asset(uint64_t asset_id, name collection_name, name schema_name, name new_asset_owner) {
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

void diploma::craft(name player, const vector<uint64_t> &asset_ids) {
    check(asset_ids.size() == 2, "For craft you need 2 pets");
    vector<Animal> pets {animal_repository.read_animal(_self, asset_ids.front()), animal_repository.read_animal(_self, asset_ids.back())};

    check(pets.front().get_rarity() == pets.back().get_rarity(), "Pets must be with the same rarity");

    crafts_or_births.emplace(_self, [&](craft_or_birth_record &rec) {
        rec.id = crafts_or_births.available_primary_key();
        rec.player = player;
        rec.rarity = pets.front().get_rarity();
        rec.first_asset_id = asset_ids.front();
        rec.second_asset_id = asset_ids.back();
        rec.type = CRAFT;
    });

    request_random(crafts_or_births.available_primary_key() - 1);
}

void diploma::oh_my_godd__a_child(name player, const vector<uint64_t> &asset_ids) {
    check(asset_ids.size() == 2, "For the birth of a child you need 2 pets");
    vector<Animal> pets {animal_repository.read_animal(_self, asset_ids.front()), animal_repository.read_animal(_self, asset_ids.back())};

    check(pets.front().get_gender() != pets.back().get_gender(), "Pets must be of different genders");
    check(pets.front().get_species() == pets.back().get_species(), "Pets must be with the same species");

    crafts_or_births.emplace(_self, [&](craft_or_birth_record &rec) {
        rec.id = crafts_or_births.available_primary_key();
        rec.player = player;
        rec.rarity = pets.front().get_rarity();
        rec.first_asset_id = asset_ids.front();
        rec.second_asset_id = asset_ids.back();
        rec.type = BIRTH;
    });

    request_random(crafts_or_births.available_primary_key() - 1);
}

void diploma::receiverand(uint64_t customer_id, checksum256 random_value) {
    require_auth("orng.wax"_n);
    Randomizer random(random_value.extract_as_byte_array());

    auto random_number = random.next<uint64_t>();

    auto iter = crafts_or_births.find(customer_id);
    check(iter != crafts_or_births.end(), "There is no such craft or birth");

    if (iter->type == CRAFT) {
        continue_craft(customer_id, random_number);
    } else {
        continue_birth(customer_id, random_number);
    }
}

uint64_t read_uint64(const checksum256 &value) {
    auto byte_array = value.extract_as_byte_array();

    uint64_t int_value = 0;
    for (int i = 0; i < 8; i++) {
        int_value <<= 8;
        int_value |= (uint64_t) byte_array[i];
    }

    return int_value;
}

void diploma::request_random(uint64_t id) {
    auto size = transaction_size();
    char buf[size];

    auto read = read_transaction(buf, size);
    check(size == read, "read_transaction() has failed.");

    auto tx_signing_value = sha256(buf, size);

    uint64_t random_int = read_uint64(tx_signing_value) ^id;

    action(
            {_self, "active"_n},
            "orng.wax"_n,
            "requestrand"_n,
            std::tuple<uint64_t, uint64_t, name>{id, random_int, _self}
    ).send();
}

void diploma::continue_craft(uint64_t id, uint64_t random_number) {
    auto iter = crafts_or_births.find(id);
    check(iter != crafts_or_births.end(), "Oops...");
    check(iter->type == CRAFT, "operator if doesn't work");

    bool chance = random_number % 100 >= 50;
    uint64_t rarity;
    if (chance) {
        rarity = iter->rarity + 1;
    } else {
        rarity = iter->rarity;
    }

    if (iter->rarity == LEGENDARY) {
        rarity = LEGENDARY;
    }

    auto animals = get_animals(rarity);
    uint64_t random_pet_id = random_number % animals.available_primary_key();
    auto animal_itr = animals.begin();

    auto i = 0;
    while (i < random_pet_id) {
        animal_itr++;
        ++i;
    }

    int32_t template_id;

    if (chance) {
        template_id = animal_itr->girl_template_id;
    } else {
        template_id = animal_itr->boy_template_id;
    }

    auto player_info_t = get_player_info(iter->player);
    auto player_info = player_info_t.get_or_default();

    player_info.newborns.push_back(template_id);
    player_info_t.set(player_info, _self);

    aa::burn_asset(_self, iter->first_asset_id);
    aa::burn_asset(_self, iter->second_asset_id);
}

string get_lowercase_name(const string& name) {
    string result;
    result.resize(name.size(), '\0');
    int i = 0;
    for (auto c: name) {
        if (c == ' ') {
            break;
        }

        result[i++] = (char) tolower(c);
    }

    return result;
}

void diploma::on_new_template(int32_t template_id, name collection_name, name schema_name) {
    if (collection_name != COLLECTION) {
        return;
    }

    if (schema_name != ANIMALS) {
        return;
    }

    aa::templates_t templates = aa::get_templates(collection_name);
    auto template_itr = templates.get(template_id, ("There is no template with id " + to_string(template_id)).c_str());

    aa::schemas_t schemas = aa::get_schemas(collection_name);
    const auto& schema_itr = schemas.get(schema_name.value, "There is no such schema");

    AnimalAttributes attributes;
    aa::deserialize(template_itr.immutable_serialized_data, schema_itr.format, attributes);

    auto rarity = get_animal_rarity(attributes.rarity);
    name animal_name = name(get_lowercase_name(attributes.name));

    auto animals = get_animals(rarity);
    auto iter = animals.find(animal_name.value);

    if (iter == animals.end()) {
        animals.emplace(_self, [&](animal_record &rec) {
            rec.name = animal_name;
            if (attributes.gender == "Boy") {
                rec.boy_template_id = template_id;
            } else {
                rec.girl_template_id = template_id;
            }
        });
    } else {
        animals.modify(iter, _self, [&](animal_record &rec) {
            if (attributes.gender == "Boy") {
                rec.boy_template_id = template_id;
            } else {
                rec.girl_template_id = template_id;
            }
        });
    }
}

void diploma::continue_birth(uint64_t id, uint64_t random_number) {
    auto iter = crafts_or_births.find(id);
    check(iter != crafts_or_births.end(), "Oopsie...");
    check(iter->type == BIRTH, "operator if doesn't work");

    bool chance = random_number % 100 >= 50;
    int32_t template_id;

    auto parent = animal_repository.read_animal(_self, iter->first_asset_id);
    name animal_name = name(get_lowercase_name(parent.get_name()));

    auto animals = get_animals(iter->rarity);
    auto animal_itr = animals.find(animal_name.value);

    if (chance) {
        template_id = animal_itr->girl_template_id;
    } else {
        template_id = animal_itr->boy_template_id;
    }

    auto player_info_t = get_player_info(iter->player);
    auto player_info = player_info_t.get_or_default();

    player_info.newborns.push_back(template_id);
    player_info_t.set(player_info, _self);

    aa::burn_asset(_self, iter->first_asset_id);
    aa::burn_asset(_self, iter->second_asset_id);
}
