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
            , food_repository(creator) {}

    [[eosio::on_notify("atomicassets::transfer")]]
    void on_asset_transfer(
            name from,
            name to,
            const vector <uint64_t> &asset_ids,
            const string &memo
    );

    [[eosio::on_notify("atomicassets::logburnasset")]]
    void on_burn_asset(name owner, uint64_t asset_id, name collection_name, name schema);

    [[eosio::on_notify("atomicassets::logmint")]]
    void on_mint_asset(uint64_t asset_id, name authorized_minter, name collection_name, name schema_name, int32_t template_id, name new_asset_owner);

    [[eosio::action("claim.token")]]
    void claim_balance(name player);

    [[eosio::action("feed")]]
    void feed(name player, uint64_t pet_asset_id, uint64_t food_asset_id);

    [[eosio::action("go.home")]]
    void lets_go_home(name player, uint64_t asset_id);
private:

    TABLE player_info_record {
        vector<uint64_t> foods;
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

    void set_satisfaction_points(Animal &animal, uint32_t walk_duration = 0, uint64_t food_healthiness = 0);
    asset calculate_walking_reward(int rarity, uint32_t duration);
    void transfer_handler(name from, name to, const vector<uint64_t>& asset_ids, const string& memo);

    AnimalRepository animal_repository;
    FoodRepository food_repository;
};