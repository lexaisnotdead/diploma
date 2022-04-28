#pragma once
#include <assets/asset_repository.hpp>
#include <animals.hpp>

class AnimalRepository {
    const eosio::name _self;
public:
    inline AnimalRepository(eosio::name self) : _self(self) {}

    Animal read_animal(eosio::name owner, uint64_t asset_id);

    Animal read_animal(const Asset &animal);

    void write_animal(eosio::name owner, const Animal &animal);
};