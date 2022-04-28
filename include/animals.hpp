#pragma once

#include <string>
#include <utility>
#include <map>

using namespace std;

#define COMMON 1
#define UNCOMMON 2
#define RARE 3
#define EPIC 4
#define LEGENDARY 5

#define offset(F) offsetof(AnimalAttributes, F)

struct AnimalAttributes {
    string name;
    string rarity;

    AnimalAttributes() = default;
    AnimalAttributes(const AnimalAttributes &other) = default;

    static const map<string, size_t> by_name;
};

inline const map<string, size_t> AnimalAttributes::by_name = {
        {"name", offset(name)},
        {"Rarity", offset(rarity)},
};

#undef offset


#define offset(F) offsetof(AnimalMutableAttributes, F)

struct AnimalMutableAttributes {
    uint64_t satisfaction_points;

    AnimalMutableAttributes() = default;
    AnimalMutableAttributes(const AnimalMutableAttributes &other) = default;

    static const map<string, size_t> by_name;
};

inline const map<string, size_t> AnimalMutableAttributes::by_name = {
        {"Satisfaction Points", offset(satisfaction_points)},
};

#undef offset


class Animal {
    string name;
    uint64_t asset_id;
    int32_t template_id;
    int rarity;

    AnimalMutableAttributes attributes;

public:
    inline Animal() = default;
    inline Animal(const Animal &other) = default;
    inline Animal &operator=(const Animal &other) = default;

    inline Animal(string name, uint64_t asset_id, int32_t template_id, int rarity, AnimalMutableAttributes const &attributes) :
    name(std::move(name)), asset_id(asset_id), template_id(template_id), rarity(rarity), attributes(attributes) {}

    string const &get_name() { return name; }

    [[nodiscard]] uint64_t get_asset_id() const { return asset_id; }

    [[nodiscard]] int get_rarity() const { return rarity; }

    AnimalMutableAttributes &get_attributes() { return attributes; }

    [[nodiscard]] AnimalMutableAttributes const &get_attributes() const { return attributes; }

    [[nodiscard]] bool is_alive() const { return attributes.satisfaction_points; }
};