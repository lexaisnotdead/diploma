#pragma once

#include <eosio/crypto.hpp>
#include <array>

#define BLOCK_SIZE 32L

inline void sha256(vector<uint8_t> const &input, array<uint8_t, 32> &output) {
    auto result = eosio::sha256(reinterpret_cast<const char *>(input.data()), input.size());
    output = result.extract_as_byte_array();
}

class Randomizer {
    std::array<uint8_t, BLOCK_SIZE> seed;
    std::array<uint8_t, BLOCK_SIZE> random_bytes;
    size_t offset;
    int index;

    inline void update_bucket() {
        index++;
        std::vector<uint8_t> buf(BLOCK_SIZE + sizeof(int));
        std::copy(random_bytes.begin(), random_bytes.end(), buf.begin());
        std::copy(&index, &index + 4, buf.begin() + BLOCK_SIZE);
        sha256(buf, random_bytes);
        offset = 0;
    }

    inline size_t write(uint8_t *p, size_t size) {
        if (offset >= random_bytes.size()) {
            update_bucket();
        }
        size_t read = min(size, random_bytes.size() - offset);
        copy(random_bytes.begin() + offset, random_bytes.begin() + offset + read, p);
        offset += read;
        return read;
    }


public:
    explicit inline Randomizer (const std::array<uint8_t, BLOCK_SIZE> &random_value) : random_bytes(random_value), seed(random_value), offset(0), index(0) {}

    Randomizer (const Randomizer &copy) = default;

    template<typename T>
    inline T next() {
        T result;

        auto *p = (uint8_t *) &result;
        size_t i = 0;
        while (i < sizeof(T)) {
            size_t written = write(p, sizeof(T) - i);
            i += written;
            p += written;
        }

        return result;
    }
};