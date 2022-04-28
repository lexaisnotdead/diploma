#pragma once

#include <string>
#include <utility>
#include <map>

using namespace std;

#define offset(F) offsetof(FoodAttributes, F)

struct FoodAttributes {
    string name;
    uint64_t healthiness;

    FoodAttributes() = default;
    FoodAttributes(const FoodAttributes &other) = default;

    static const map<string, size_t> by_name;
};

inline const map<string, size_t> FoodAttributes::by_name = {
        {"name", offset(name)},
        {"healthiness", offset(healthiness)},
};

#undef offset


class Food {
    string name;
    uint64_t asset_id;
    uint64_t healthiness;

public:
    inline Food() = default;
    inline Food(const Food &other) = default;
    inline Food &operator=(const Food &other) = default;

    inline Food(string name, uint64_t asset_id, uint64_t healthiness) :
    name(std::move(name)), asset_id(asset_id), healthiness(healthiness) {}

    string const &get_name() { return name; }

    [[nodiscard]] uint64_t get_asset_id() const { return asset_id; }

    [[nodiscard]] uint64_t get_healthiness() const { return healthiness; }
};