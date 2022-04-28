#pragma once

#include <cstdint>
#include <vector>
#include <variant>
#include <map>
#include <string>

using namespace std;

namespace aa {
    //Custom vector types need to be defined because otherwise a bug in the ABI serialization
    //would cause the ABI to be invalid
    typedef std::vector <int16_t>     v_int16_t;
    typedef std::vector <int32_t>     v_int32_t;
    typedef std::vector <int64_t>     v_int64_t;
    typedef std::vector <uint8_t>     v_uint8_t;
    typedef std::vector <uint16_t>    v_uint16_t;
    typedef std::vector <uint32_t>    v_uint32_t;
    typedef std::vector <uint64_t>    v_uint64_t;
    typedef std::vector <float>       v_float_t;
    typedef std::vector <double>      v_double_t;
    typedef std::vector <std::string> v_string_t;

    typedef std::variant <\
        int8_t, int16_t, int32_t, int64_t, \
        uint8_t, uint16_t, uint32_t, uint64_t, \
        float, double, std::string, \
        std::vector<int8_t>, v_int16_t, v_int32_t, v_int64_t, \
        v_uint8_t, v_uint16_t, v_uint32_t, v_uint64_t, \
        v_float_t, v_double_t, v_string_t \
    > attribute_t;

    typedef std::map <std::string, attribute_t> attribute_map;

    struct format {
        string name;
        string type;

        format() = default;
        format(format const &) = default;
    };

    static constexpr uint64_t RESERVED = 4;

    attribute_map build_map(uint8_t const *data_attrs, vector<format> const &formats, map<string, size_t> const &names);
    void read_map(attribute_map const &attr_map, vector<format> const &formats, map<string, size_t> const &names, uint8_t *data);

    template <class T>
    inline attribute_map build_map(T &data, vector<format> const &formats) {
        const map<string, size_t> &names = data.by_name;
        uint8_t *data_argument = (uint8_t*) &data;
        return build_map(data_argument, formats, names);
    }

    template <class T>
    inline void read_map(attribute_map const &attr_map, T &data, vector<format> const &formats) {
        const map<string, size_t> &names = data.by_name;
        uint8_t *data_argument = (uint8_t*) &data;
        read_map(attr_map, formats, names, data_argument);
    }
}