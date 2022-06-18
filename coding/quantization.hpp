#pragma once

#include <cstdint>
#include <vector>

namespace coding {

/**
 * @brief kwantyzacja rownomierna
 *
 * @param data dane do skwantyzowania
 * @param bits 0 do 8 ilosc informacji na bajt w bitach
 * @return std::vector<uint8_t> skwantyzowane dane
 */
inline auto uniform_quantization(const std::vector<uint8_t>& data, uint32_t bits) -> std::vector<uint8_t>
{
    std::vector<uint8_t> result;

    auto quantize = [&bits](uint32_t val) -> uint8_t{
        uint32_t upper = 256;
        uint32_t lower = 0;
        for (uint32_t i {}; i < bits; ++i) {
            auto mid = lower + (upper - lower) / 2;
            if (val < mid) {
                upper = mid;
            } else {
                lower = mid;
            }
        }

        uint32_t result = lower + (upper - lower) / 2;
        return result;
    };
    auto size = data.size();

    result.resize(size);
    for (decltype(size) i {}; i < size; ++i) {
        result[i] = quantize(data[i]);
    }

    return result;
}
}