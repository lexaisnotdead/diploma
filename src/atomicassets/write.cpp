#include <atomicassets/write.hpp>
#include <atomicassets/base58.hpp>
#include <atomicassets/zigzag.hpp>

using namespace std;

namespace aa {
    int write_variant_bytes(vector<uint8_t> &dst, uint64_t number, uint64_t original_bytes) {
        if (original_bytes < 8) {
            uint64_t bitmask = ((uint64_t) 1 << original_bytes * 8) - 1;
            number &= bitmask;
        }
        int bytes = 1;
        while (number >= 128) {
            // sets msb, stores remainder in lower bits
            dst.push_back((uint8_t) (128 + number % 128));
            number /= 128;
            bytes++;
        }
        dst.push_back((uint8_t) number);

        return bytes;
    }

    //It is expected that the number is smaller than 2^byte_amount
    int write_bytes(vector<uint8_t> &dst, uint64_t number, uint64_t byte_amount) {
        int i = 0;
        for (; i < byte_amount; i++) {
            dst.push_back((uint8_t) number % 256);
            number /= 256;
        }
        return i;
    }

    size_t encode_string(uint8_t const *src, vector<uint8_t> &dst) {
        auto p = (string *) src;

        write_variant_bytes(dst, p->length(), sizeof(p->length()));
        dst.insert(dst.end(), p->begin(), p->end());

        return sizeof(string);
    }

    size_t encode_ipfs(uint8_t const *src, vector<uint8_t> &dst) {
        auto p = (string *) src;

        vector<uint8_t> tmp;
        DecodeBase58(*p, tmp);
        write_variant_bytes(dst, tmp.size(), sizeof(tmp.size()));
        dst.insert(dst.end(), tmp.begin(), tmp.end());

        return sizeof(string);
    }

    template<typename T>
    size_t encode_uint(uint8_t const *src, vector<uint8_t> &dst) {
        auto p = (T *) src;
        write_variant_bytes(dst, *p, sizeof(T));
        return sizeof(T);
    }

    template<typename T>
    size_t encode_fixed(uint8_t const *src, vector<uint8_t> &dst) {
        auto p = (T *) src;
        write_bytes(dst, *p, sizeof(T));
        return sizeof(T);
    }

    template<typename T>
    size_t encode_int(uint8_t const *src, vector<uint8_t> &dst) {
        auto p = (T *) src;
        write_bytes(dst, zigzag_encode(*p), sizeof(T));
        return sizeof(T);
    }

    typedef size_t (*encode_func)(uint8_t const *src, vector<uint8_t> &dst);

    const map<string, encode_func> encoders{
            {"string",  encode_string},
            {"image",   encode_string},
            {"ipfs",    encode_ipfs},
            {"uint8",   encode_uint<uint8_t>},
            {"uint16",  encode_uint<uint16_t>},
            {"uint32",  encode_uint<uint32_t>},
            {"uint64",  encode_uint<uint64_t>},
            {"int8",    encode_int<int8_t>},
            {"int16",   encode_int<int16_t>},
            {"int32",   encode_int<int32_t>},
            {"int64",   encode_int<int64_t>},
            {"fixed8",  encode_fixed<uint8_t>},
            {"fixed16", encode_fixed<uint16_t>},
            {"fixed32", encode_fixed<uint32_t>},
            {"fixed64", encode_fixed<uint64_t>},
    };

    void serialize(uint8_t const *data, vector<format> const &formats, const map<string, size_t> &names,
                   vector<uint8_t> &dst) {
        size_t size = formats.size();
        for (int i = 0; i < size; i ++) {
            auto const &format = formats[i];
            auto itr = names.find(format.name);
            if (itr == names.end()) {
                continue;
            }
            dst.push_back(i + RESERVED);
            encode_func encoder = encoders.at(format.type);
            size_t offset = itr->second;
            encoder(data + offset, dst);
        }
    }
}
