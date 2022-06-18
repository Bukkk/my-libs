#pragma once

#include <cstdint>
#include <cmath>
#include <vector> // na przyszlosc mozna sie pobawic w koncepty jak bedzie potrzeba oddzielic od vectora

namespace coding::statistics {

inline auto entropy(const std::vector<unsigned char>& set) -> double
{
    auto const size = set.size();

    uint64_t amount[0x100] = {};
    for (size_t i = 0; i < size; ++i) {
        ++amount[set[i]];
    }

    auto const log_size = std::log2(size);
    double entropy = 0;
    for (uint32_t x = 0; x < 0x100; ++x) {

        if (amount[x] != 0) {
            auto diff = amount[x] * (log_size - std::log2(amount[x]));
            entropy += diff;
        }
    }
    entropy /= size;

    return entropy;
}

inline auto average_coding_length(std::size_t coded_size, std::size_t orginal_size) -> double
{
    return 8. * coded_size / orginal_size;
}

inline auto compression_ratio(std::size_t coded_size, std::size_t orginal_size) -> double
{
    return 1. * coded_size / orginal_size;
}

}