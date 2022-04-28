#pragma once

#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/asset.hpp>
#include <atomicassets/atomicdata.hpp>

using namespace eosio;
using namespace std;

namespace aa {
    class [[eosio::contract]] atomicassets {
    public:
        [[eosio::action]]
        void mintasset(
                name authorized_minter,
                name collection_name,
                name schema_name,
                int32_t template_id,
                name new_asset_owner,
                attribute_map immutable_data,
                attribute_map mutable_data,
                vector<asset> tokens_to_back
        );

        typedef action_wrapper<"mintasset"_n, &atomicassets::mintasset> mintasset_wrapper;

        [[eosio::action]]
        void burnasset(name owner, uint64_t asset_id);
        typedef action_wrapper<"burnasset"_n, &atomicassets::burnasset> burnasset_wrapper;

        [[eosio::action]]
        void transfer(name from, name to,  vector<uint64_t> asset_ids, string memo);
        typedef action_wrapper<"transfer"_n, &atomicassets::transfer> transfer_wrapper;
    };

    static constexpr name ATOMICASSETS_ACCOUNT = "atomicassets"_n;
    static constexpr double MAX_MARKET_FEE = 0.15;

    struct collections_s {
        name collection_name;
        name author;
        bool allow_notify;
        vector<name> authorized_accounts;
        vector<name> notify_accounts;
        double market_fee;
        vector<uint8_t> serialized_data;

        [[maybe_unused]]
        uint64_t primary_key() const { return collection_name.value; };
    };

    typedef multi_index<"collections"_n, collections_s> collections_t;


    //Scope: collection_name
    struct schemas_s {
        name schema_name;
        vector<format> format;

        [[maybe_unused]]
        uint64_t primary_key() const { return schema_name.value; }
    };

    typedef multi_index<"schemas"_n, schemas_s> schemas_t;


    //Scope: collection_name
    struct templates_s {
        int32_t template_id;
        name schema_name;
        bool transferable;
        bool burnable;
        uint32_t max_supply;
        uint32_t issued_supply;
        vector<uint8_t> immutable_serialized_data;

        [[maybe_unused]]
        uint64_t primary_key() const { return (uint64_t) template_id; }
    };

    typedef multi_index<"templates"_n, templates_s> templates_t;


    //Scope: owner
    struct assets_s {
        uint64_t asset_id;
        name collection_name;
        name schema_name;
        int32_t template_id;
        name ram_payer;
        vector<asset> backed_tokens;
        vector<uint8_t> immutable_serialized_data;
        vector<uint8_t> mutable_serialized_data;

        [[maybe_unused]]
        uint64_t primary_key() const { return asset_id; };
    };

    typedef multi_index<"assets"_n, assets_s> assets_t;


    struct offers_s {
        uint64_t offer_id;
        name sender;
        name recipient;
        vector<uint64_t> sender_asset_ids;
        vector<uint64_t> recipient_asset_ids;
        string memo;
        name ram_payer;

        [[maybe_unused]]
        uint64_t primary_key() const { return offer_id; };

        uint64_t by_sender() const { return sender.value; };

        uint64_t by_recipient() const { return recipient.value; };
    };

    typedef multi_index<"offers"_n, offers_s,
            indexed_by<"sender"_n, const_mem_fun<offers_s, uint64_t, &offers_s::by_sender>>,
            indexed_by<"recipient"_n, const_mem_fun<offers_s, uint64_t, &offers_s::by_recipient>>>
            offers_t;

    struct balances_s {
        name owner;
        vector<asset> quantities;

        [[maybe_unused]]
        uint64_t primary_key() const { return owner.value; };
    };

    typedef multi_index<"balances"_n, balances_s> balances_t;


    struct config_s {
        uint64_t asset_counter = 1099511627776; //2^40
        int32_t template_counter = 1;
        uint64_t offer_counter = 1;
        vector<format> collection_format = {};
        vector<extended_symbol> supported_tokens = {};
    };
    typedef singleton<"config"_n, config_s> config_t;

    struct tokenconfigs_s {
        name standard = name("atomicassets");
        std::string version = string("1.2.3");
    };
    typedef singleton<"tokenconfigs"_n, tokenconfigs_s> tokenconfigs_t;


    extern collections_t collections;//  = collections_t(ATOMICASSETS_ACCOUNT, ATOMICASSETS_ACCOUNT.value);
    extern offers_t offers;//       = offers_t(ATOMICASSETS_ACCOUNT, ATOMICASSETS_ACCOUNT.value);
    extern balances_t balances;//     = balances_t(ATOMICASSETS_ACCOUNT, ATOMICASSETS_ACCOUNT.value);
    extern config_t config;//       = config_t(ATOMICASSETS_ACCOUNT, ATOMICASSETS_ACCOUNT.value);
    extern tokenconfigs_t tokenconfigs;// = tokenconfigs_t(ATOMICASSETS_ACCOUNT, ATOMICASSETS_ACCOUNT.value);

    inline assets_t get_assets(name acc) {
        return assets_t(ATOMICASSETS_ACCOUNT, acc.value);
    }

    inline schemas_t get_schemas(name collection_name) {
        return schemas_t(ATOMICASSETS_ACCOUNT, collection_name.value);
    }

    inline templates_t get_templates(name collection_name) {
        return templates_t(ATOMICASSETS_ACCOUNT, collection_name.value);
    }

    inline void mint_asset(
            //name caller_name,
            name authorized_minter,
            name collection_name,
            name schema_name,
            int32_t template_id,
            name new_asset_owner,
            attribute_map immutable_data,
            attribute_map mutable_data,
            vector<asset> tokens_to_back) {
        atomicassets::mintasset_wrapper action(ATOMICASSETS_ACCOUNT, {authorized_minter, "active"_n});
        action.send(authorized_minter, collection_name, schema_name, template_id, new_asset_owner, immutable_data, mutable_data, tokens_to_back);
    }

    inline void burn_asset(name owner, uint64_t asset_id) {
        atomicassets::burnasset_wrapper action(ATOMICASSETS_ACCOUNT, {owner, "active"_n});
        action.send(owner, asset_id);
    }

    inline void transfer_asset(name from, name to, vector<uint64_t> asset_ids, string memo) {
        atomicassets::transfer_wrapper action(ATOMICASSETS_ACCOUNT, {from, "active"_n});
        action.send(from, to, asset_ids, memo);
    }
};
