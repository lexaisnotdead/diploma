#pragma once

#include <cstdint>
#include <vector>
#include <variant>
#include <map>
#include <string>
#include <atomicassets/atomicdata.hpp>

using namespace std;

namespace aa {
    void deserialize(vector<uint8_t> const &data, vector<format> const &formats, map<string, size_t> const &names, uint8_t *dst);

    template <class T>
    inline void deserialize(vector<uint8_t> const &data, vector<format> const &formats, T &dst_attrs) {
        const map<string, size_t> &names = dst_attrs.by_name;
        auto *dst = (uint8_t *) &dst_attrs;
        deserialize(data, formats, names, dst);
    }
}