#include <assets/animal_repository.hpp>

Animal AnimalRepository::read_animal(eosio::name owner, uint64_t asset_id) {
    Asset animal = get_asset(owner, asset_id);

    return read_animal(animal);
}

int get_animal_rarity(const string& rarity) {
    char ch = rarity.at(0);

    switch (ch) {
        case 'C':
            return COMMON;
        case 'U':
            return UNCOMMON;
        case 'R':
            return RARE;
        case 'E':
            return EPIC;
        case 'L':
            return LEGENDARY;
        default:
            check(false, "Unsupported rarity '" + rarity + "'.");
    }
}

Animal AnimalRepository::read_animal(const Asset &animal) {
    check(animal.schemas.schema_name == ANIMALS, "Wrong asset schema, expected " + ANIMALS.to_string() + " but got " + animal.schemas.schema_name.to_string() + "; asset: " + to_string(animal.asset.asset_id));

    AnimalAttributes attributes;
    AnimalMutableAttributes mutable_attributes{};

    aa::deserialize(animal.templates.immutable_serialized_data, animal.schemas.format, attributes);
    aa::deserialize(animal.asset.mutable_serialized_data, animal.schemas.format, mutable_attributes);

    int rarity = get_animal_rarity(attributes.rarity);
    bool gender = attributes.gender == "Boy";

    return Animal(attributes.name, animal.asset.asset_id, attributes.species, rarity, gender, mutable_attributes);
}

void AnimalRepository::write_animal(eosio::name owner, const Animal &animal) {
    auto assets = aa::get_assets(owner);
    auto const &asset = assets.get(animal.get_asset_id(), "Wrong animal asset id");

    aa::schemas_t schemas = aa::get_schemas(asset.collection_name);
    auto const &schema = schemas.get(asset.schema_name.value, "There is no asset's schema.");

    auto const &attributes = animal.get_attributes();

    aa::attribute_map attr_map = aa::build_map(attributes, schema.format);

    action(
            permission_level{_self, "active"_n},
            aa::ATOMICASSETS_ACCOUNT,
            "setassetdata"_n,
            make_tuple(_self, owner, asset.asset_id, attr_map)
    ).send();
}
