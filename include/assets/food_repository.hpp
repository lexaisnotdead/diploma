#pragma once
#include <assets/asset_repository.hpp>
#include <foods.hpp>

class FoodRepository {
    const eosio::name _self;
public:
    inline FoodRepository(eosio::name self) : _self(self) {}

    Food read_food(const Asset &food);

    Food read_food(eosio::name owner, uint64_t asset_id);
};