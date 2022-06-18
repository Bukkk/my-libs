#pragma once

#include <cstdint>
#include <vector>

namespace coding::misc {
inline auto vector_cast(const std::vector<unsigned char>& char_vector) -> std::vector<bool>
{
    std::vector<bool> bool_vector {};
    bool_vector.reserve(8 * char_vector.size());

    for (std::size_t byte_id {}; byte_id < char_vector.size(); ++byte_id) {
        for (uint32_t bit_id = 8; bit_id > 0; --bit_id) {
            bool val = char_vector[byte_id] & (1 << (bit_id - 1));
            bool_vector.push_back(val);
        }
    }

    return bool_vector;
}

inline auto vector_cast(const std::vector<bool>& bool_vector, bool padding = false) -> std::vector<unsigned char>
{
    std::vector<unsigned char> char_vector(bool_vector.size() / 8 + (bool_vector.size() % 8 ? 1 : 0));

    std::size_t i {};
    for (; i < bool_vector.size(); ++i) {
        std::size_t byte_id = i / 8;
        uint32_t bit_id = 8 - i % 8;

        unsigned char val = bool_vector[i] ? (1 << (bit_id - 1)) : 0;
        char_vector[byte_id] |= val;
    }

    for (; i % 8 != 0 ; ++i) {
        std::size_t byte_id = i / 8;
        uint32_t bit_id = 8 - i % 8;

        unsigned char val = padding ? (1 << (bit_id - 1)) : 0;
        char_vector[byte_id] |= val;
    }

    return char_vector;
}
}