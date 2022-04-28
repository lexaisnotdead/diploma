#include <atomicassets/atomicdata.hpp>

using namespace std;

namespace aa {
    attribute_t string_to_attribute(uint8_t const *src) {
        return attribute_t(*((string *) src));
    }

    template<typename T>
    void attribute_to_T(attribute_t const &attribute, uint8_t *dst) {
        *((T *) dst) = std::get<T>(attribute);
    }

    template<typename T>
    attribute_t int_to_attribute(uint8_t const *src) {
        return attribute_t(*((T *) src));
    }

    typedef attribute_t (*to_attribute_func)(uint8_t const *src);
    typedef void (*from_attribute_func)(attribute_t const & attribute, uint8_t *dst);

    const map<string, to_attribute_func> to_attribute {
            {"string",  string_to_attribute},
            {"image",   string_to_attribute},
            {"uint8",   int_to_attribute<uint8_t>},
            {"uint16",  int_to_attribute<uint16_t>},
            {"uint32",  int_to_attribute<uint32_t>},
            {"uint64",  int_to_attribute<uint64_t>},
            {"int8",    int_to_attribute<int8_t>},
            {"int16",   int_to_attribute<int16_t>},
            {"int32",   int_to_attribute<int32_t>},
            {"int64",   int_to_attribute<int64_t>},
            {"fixed8",  int_to_attribute<uint8_t>},
            {"fixed16", int_to_attribute<uint16_t>},
            {"fixed32", int_to_attribute<uint32_t>},
            {"fixed64", int_to_attribute<uint64_t>},
    };

    const map<string, from_attribute_func> from_attribute {
            {"string",  attribute_to_T<string>},
            {"image",   attribute_to_T<string>},
            {"uint8",   attribute_to_T<uint8_t>},
            {"uint16",  attribute_to_T<uint16_t>},
            {"uint32",  attribute_to_T<uint32_t>},
            {"uint64",  attribute_to_T<uint64_t>},
            {"int8",    attribute_to_T<int8_t>},
            {"int16",   attribute_to_T<int16_t>},
            {"int32",   attribute_to_T<int32_t>},
            {"int64",   attribute_to_T<int64_t>},
            {"fixed8",  attribute_to_T<uint8_t>},
            {"fixed16", attribute_to_T<uint16_t>},
            {"fixed32", attribute_to_T<uint32_t>},
            {"fixed64", attribute_to_T<uint64_t>},
    };

    attribute_map build_map(uint8_t const *data, vector<format> const &formats, map<string, size_t> const &names) {
        attribute_map attr_map = {};

        for (auto const &format : formats) {
            to_attribute_func converter = to_attribute.at(format.type);
            if (names.find(format.name) == names.end()) {
                continue;
            }
            size_t offset = names.at(format.name);
            attr_map[format.name] = converter(data + offset);
        }

        return attr_map;
    }

    void read_map(attribute_map const &attr_map, vector<format> const &formats, map<string, size_t> const &names, uint8_t *data) {

        for (auto const &format : formats) {
            if (attr_map.find(format.name) == attr_map.end()) {
                continue;
            }
            if (names.find(format.name) == names.end()) {
                continue;
            }
            from_attribute_func converter = from_attribute.at(format.type);
            size_t offset = names.at(format.name);
            converter(attr_map.at(format.name), data + offset);
        }
    }
}
