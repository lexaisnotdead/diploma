#include <assets/food_repository.hpp>

Food FoodRepository::read_food(const Asset &food) {
    check(food.schemas.schema_name == FOODS, "Wrong asset schema, expected " + FOODS.to_string() + " but got " + food.schemas.schema_name.to_string() + "; asset: " + to_string(food.asset.asset_id));
    FoodAttributes attributes;

    aa::deserialize(food.templates.immutable_serialized_data, food.schemas.format, attributes);

    return Food(attributes.name, food.asset.asset_id, attributes.healthiness);
}

Food FoodRepository::read_food(eosio::name owner, uint64_t asset_id) {
    Asset food = get_asset(owner, asset_id);

    return read_food(food);
}