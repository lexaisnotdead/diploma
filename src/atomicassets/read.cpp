#include <atomicassets/read.hpp>
#include <atomicassets/base58.hpp>
#include <atomicassets/zigzag.hpp>
#include <eosio/eosio.hpp>
using namespace std;

namespace aa {
    /**
     * Read variable bytes to uint64
     * @param itr Readable bytes array pointer.
     * @return The value which was read.
     */
    uint64_t read_variant(vector<uint8_t>::const_iterator &itr) {
        uint64_t result = 0;
        uint8_t b;
        int i = 0;
        do {
            eosio::check(i <= 8, "only 8 bytes maximum might be read.");
            b = *itr;
            result |= ((uint64_t) (b & 0x7f) << (i * 7));
            itr++;
            i++;
        } while ((b & 0x80) != 0);

        return result;
    }

    /**
     * Read fixed length bytes.
     * @param itr Readable bytes array pointer
     * @param length Bytes to read.
     * @return The value which was read.
     */
    uint64_t read_fixed(vector<uint8_t>::const_iterator &itr, uint64_t length) {
        uint64_t number = 0;

        for (uint64_t i = 0; i < length; i++) {
            number |= ((uint64_t) *itr) << (i * 8);
            itr++;
        }

        return number;
    }

    template<class T>
    T read_bytes(vector<uint8_t>::const_iterator &itr) {
        auto length = (vector<uint8_t>::const_iterator::difference_type) read_variant(itr);
        T result(itr, itr + length);
        itr += length;
        return result;
    }

    template<class T>
    size_t decode_bytes(uint8_t *dst, vector<uint8_t>::const_iterator &itr) {
        T tmp = read_bytes<T>(itr);

        if (dst != nullptr) {
            ((T *) dst)->assign(tmp);
        }

        return sizeof(T);
    }

    size_t decode_ipfs(uint8_t *dst, vector<uint8_t>::const_iterator &itr) {
        auto bytes = read_bytes<vector<uint8_t>>(itr);
        string tmp = EncodeBase58(bytes);

        if (dst != nullptr) {
            ((string *) dst)->assign(tmp);
        }
        return sizeof(string);
    }

    template<typename T>
    size_t decode_uint(uint8_t *dst, vector<uint8_t>::const_iterator &itr) {
        static_assert(((T) -1) > 0, "must be specified unsigned type");
        T stub;
        auto p = dst == nullptr ? &stub : (T *) dst;
        uint64_t value = read_variant(itr);
        if (sizeof(T) < 8) {
            eosio::check(value >> (sizeof(T) * 8) == 0, "Read data has more length than expected.");
        }
        *p = (T) value;
        return sizeof(T);
    }

    template<typename T>
    size_t decode_fixed(uint8_t *dst, vector<uint8_t>::const_iterator &itr) {
        static_assert(((T) -1) > 0, "must be specified unsigned type");
        static_assert(sizeof(T) <= 8, "must be not more than uint64");
        T stub;
        auto p = dst == nullptr ? &stub : (T *) dst;
        uint64_t value = read_fixed(itr, sizeof(T));
        *p = (T) value;
        return sizeof(T);
    }

    template<typename T>
    size_t decode_int(uint8_t *dst, vector<uint8_t>::const_iterator &itr) {
        static_assert(((T) -1) < 0, "must be specified signed type");
        T stub;
        auto p = dst == nullptr ? &stub : (T *) dst;
        uint64_t unsigned_value = read_variant(itr);
        if (sizeof(T) < 8) {
            eosio::check(unsigned_value >> (sizeof(T) * 8) == 0, "Read data has more length than expected.");
        }
        int64_t signed_value = zigzag_decode(unsigned_value);
        *p = (T) signed_value;
        return sizeof(T);
    }

    typedef size_t (*decode_func)(uint8_t *dst, vector<uint8_t>::const_iterator &itr);

    const map<string, decode_func> decoders{
            {"string",  decode_bytes<std::string>},
            {"image",   decode_bytes<std::string>},
            {"ipfs",    decode_ipfs},
            {"uint8",   decode_uint<uint8_t>},
            {"uint16",  decode_uint<uint16_t>},
            {"uint32",  decode_uint<uint32_t>},
            {"uint64",  decode_uint<uint64_t>},
            {"int8",    decode_int<int8_t>},
            {"int16",   decode_int<int16_t>},
            {"int32",   decode_int<int32_t>},
            {"int64",   decode_int<int64_t>},
            {"fixed8",  decode_fixed<uint8_t>},
            {"fixed16", decode_fixed<uint16_t>},
            {"fixed32", decode_fixed<uint32_t>},
            {"fixed64", decode_fixed<uint64_t>},
    };

    void deserialize(vector<uint8_t> const &data,
                     vector<format> const &formats,
                     map<string, size_t> const &names,
                     uint8_t *dst) {
        auto itr = data.begin();
        const size_t max_format_id = formats.size();
        while (itr != data.end()) {
            uint64_t formatId = read_variant(itr) - RESERVED;
            eosio::check (formatId < max_format_id, "Wrong formatId values.");
            auto &format = formats[formatId];
            decode_func decoder = decoders.at(format.type);
            if (names.find(format.name) == names.end()) {
                decoder(nullptr, itr);
                continue;
            }
            size_t offset = names.at(format.name);
            decoder(dst + offset, itr);
        }
    }
}
