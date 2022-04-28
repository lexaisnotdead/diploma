#pragma once

#include <cstdint>
#include <vector>
#include <variant>
#include <map>
#include <string>
#include <atomicassets/atomicdata.hpp>

using namespace std;

namespace aa {
    int write_variant_bytes(vector<uint8_t> &dst, uint64_t number, uint64_t original_bytes);

    //It is expected that the number is smaller than 2^byte_amount
    int write_bytes(vector<uint8_t> &dst, uint64_t number, uint64_t byte_amount);

//    int serialize_attribute(vector<uint8_t> &dst, const string &type, const attribute_t &attr);

//    vector <uint8_t> serialize(attribute_map attr_map, const vector <format> &format_lines);

    void serialize(uint8_t const *data, vector<format> const &formats, const map<string, size_t> &names, vector<uint8_t> &dst);

    template <class T>
    inline void serialize(T &data, vector<format> const &formats, vector<uint8_t> &dst_attrs) {
        const map<string, size_t> &names = data.by_name;
        uint8_t *data_argument = (uint8_t*) &data;
        serialize(data_argument, formats, names, dst_attrs);
    }
}