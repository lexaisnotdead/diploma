#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <eosio/transaction.hpp>

#include <diplomadef.hpp>
#include <atomicassets/atomicassets.hpp>
#include <atomicassets/atomicdata.hpp>
#include <eosio.token.hpp>
#include <assets/food_repository.hpp>
#include <assets/animal_repository.hpp>

using namespace std;
using namespace eosio;

CONTRACT diploma: public contract {
public:
    using contract :: contract;

    diploma(name creator, name code, datastream<const char *> ds)
            : contract(creator, code, ds)
            , animal_repository(creator)
            , food_repository(creator)
            , crafts_or_births(creator, creator.value) {}

    [[eosio::on_notify("atomicassets::transfer")]]
    void on_asset_transfer(
            name from,
            name to,
            const vector <uint64_t> &asset_ids,
            const string &memo
    );

    [[eosio::on_notify("atomicassets::lognewtempl")]]
    void on_new_template(int32_t template_id, name collection_name, name schema_name);

    [[eosio::on_notify("atomicassets::logburnasset")]]
    void on_burn_asset(name owner, uint64_t asset_id, name collection_name, name schema);

    [[eosio::on_notify("atomicassets::logmint")]]
    void on_mint_asset(uint64_t asset_id, name collection_name, name schema_name, name new_asset_owner);

    [[eosio::action("claim.token")]]
    void claim_balance(name player);

    [[eosio::action("feed")]]
    void feed(name player, uint64_t pet_asset_id, uint64_t food_asset_id);

    [[eosio::action("go.home")]]
    void lets_go_home(name player, uint64_t asset_id);

    ACTION receiverand(uint64_t customer_id, checksum256 random_value);
private:

    TABLE player_info_record {
        vector<uint64_t> foods;
        vector<int32_t> newborns;
        asset balance;
    };
    typedef singleton<"player.info"_n, player_info_record> player_info_table;
    typedef multi_index<"player.info"_n, player_info_record> player_info_for_abi;
    inline player_info_table get_player_info(name player) { return {_self, player.value}; }

    enum where_is_my_pet {
        AT_HOME,
        ON_A_WALK,
    };

    TABLE player_pets_record {
        uint64_t asset_id;
        uint32_t last_walk_start_ts;
        uint32_t last_walk_end_ts;
        asset walking_reward;
        uint32_t last_feed_ts;
        int status = AT_HOME;

        [[nodiscard]] uint64_t primary_key() const { return asset_id; }
    };
    typedef multi_index<"player.pets"_n, player_pets_record> player_pets_table;
    inline player_pets_table get_player_pets(name player) { return {_self, player.value}; }

    enum types {
        BIRTH,
        CRAFT,
    };

    TABLE craft_or_birth_record {
        uint64_t id;
        name player;
        uint64_t first_asset_id;
        uint64_t second_asset_id;
        uint64_t rarity;
        uint16_t type;

        [[nodiscard]] uint64_t primary_key() const { return id; }
    };
    typedef multi_index<"crafts.birth"_n, craft_or_birth_record> crafts_or_births_table;

    TABLE animal_record {
        name name;
        int32_t boy_template_id;
        int32_t girl_template_id;

        [[nodiscard]] uint64_t primary_key() const { return name.value; }
    };
    typedef multi_index<"animals"_n, animal_record> animals_table;
    inline animals_table get_animals(uint64_t rarity) { return {_self, rarity}; }

    void set_satisfaction_points(Animal &animal, uint32_t walk_duration = 0, uint64_t food_healthiness = 0);
    asset calculate_walking_reward(int rarity, uint32_t duration);
    void transfer_handler(name from, name to, const vector<uint64_t>& asset_ids, const string& memo);
    void craft(name player, const vector<uint64_t> &asset_ids);
    void oh_my_godd__a_child(name player, const vector<uint64_t>& asset_ids);
    void request_random(uint64_t id);
    void continue_craft(uint64_t id, uint64_t random_number);
    void continue_birth(uint64_t id, uint64_t random_number);

    AnimalRepository animal_repository;
    FoodRepository food_repository;
    crafts_or_births_table crafts_or_births;
};