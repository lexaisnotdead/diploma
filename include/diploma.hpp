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

    [[eosio::action("claim.token")]]
    void claim_balance(name player);

    [[eosio::action("feed")]]
    void feed(name player, uint64_t pet_asset_id, uint64_t food_asset_id);

private:

    TABLE player_info_record {
        asset balance;
    };
    typedef singleton<"player.info"_n, player_info_record> player_info_table;
    typedef multi_index<"player.info"_n, player_info_record> player_info_for_abi;
    inline player_info_table get_player_info(name player) { return {_self, player.value}; }

    TABLE player_pets_record {
        uint64_t asset_id;
        uint32_t beginning_of_a_walk;
        uint32_t end_of_a_walk;
        asset walking_reward;
        uint32_t last_feed_ts;

        [[nodiscard]] uint64_t primary_key() const { return asset_id; }
    };
    typedef multi_index<"player.pets"_n, player_pets_record> player_pets_table;
    inline player_pets_table get_player_pets(name player) { return {_self, player.value}; }

    AnimalRepository animal_repository;
    FoodRepository food_repository;
};