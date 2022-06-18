#pragma once

#include "misc.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace coding::acs {

namespace {
    inline constexpr uint64_t BITS_PRECISION = 32;
    inline constexpr uint64_t WHOLE_INTERVAL = uint64_t { 1 } << BITS_PRECISION;
    inline constexpr uint64_t HALF_INTERVAL = WHOLE_INTERVAL / 2;
    inline constexpr uint64_t QUARTER_INTERVAL = WHOLE_INTERVAL / 4;
    inline constexpr uint64_t BLOCK_SIZE = 128;
}

namespace adaptive {
    inline constexpr uint64_t MAX_OCCURENCE = QUARTER_INTERVAL - 1;
    inline constexpr uint64_t SYMBOL_AMOUNT = 256;

    struct cache {
        std::array<uint64_t, SYMBOL_AMOUNT> occurence_delta_ {};
        uint64_t occurences_sum_ {};

        void update(uint32_t symbol)
        {
            ++occurence_delta_[symbol];
            ++occurences_sum_;
        }
    };

    class model {

        std::array<uint64_t, SYMBOL_AMOUNT> occurence_ {};
        std::array<uint64_t, SYMBOL_AMOUNT> scale_from_ {};
        std::array<uint64_t, SYMBOL_AMOUNT> scale_to_ {};

        uint64_t occurences_sum_;

        void calculate_scale()
        {
            uint64_t sum = 0;
            for (size_t symbol = 0; symbol < occurence_.size(); symbol++) {
                scale_from_[symbol] = sum;
                sum += occurence_[symbol];
                scale_to_[symbol] = sum;
            }
        }

    public:
        model()
        {
            static_assert(SYMBOL_AMOUNT < MAX_OCCURENCE, "za duza liczba symboli - nie mieszcza sie w maksymalnej dokladnosci");

            for (std::size_t i = 0; i < occurence_.size(); i++)
                occurence_[i] = 1;

            occurences_sum_ = SYMBOL_AMOUNT;

            calculate_scale();
        }

        auto size()
        {
            return occurence_.size();
        }

        auto occurences_sum()
        {
            return occurences_sum_;
        }

        auto scale_from(uint32_t symbol)
        {
            return scale_from_[symbol];
        }

        auto scale_to(uint32_t symbol)
        {
            return scale_to_[symbol];
        }

        void update(uint32_t symbol)
        {
            occurence_[symbol]++;
            occurences_sum_++;

            calculate_scale();
        }

        void update(const cache& cache)
        {
            for (uint32_t symbol {}; symbol < SYMBOL_AMOUNT; ++symbol) {
                occurence_[symbol] += cache.occurence_delta_[symbol];
            }

            occurences_sum_ += cache.occurences_sum_;
            calculate_scale();
        }
    };
}

inline auto encode(const std::vector<unsigned char>& input_vector) -> std::vector<unsigned char>
{
    std::vector<unsigned char> output_vector {};

    adaptive::model model {};
    adaptive::cache cache {};

    uint64_t a = 0;
    uint64_t b = WHOLE_INTERVAL;
    uint64_t licznik = 0;

    std::vector<bool> bool_vector {};
    for (auto byte : input_vector) {

        uint32_t symbol = byte;
        uint64_t w = b - a;
        b = a + (w * model.scale_to(symbol) / model.occurences_sum());
        a = a + (w * model.scale_from(symbol) / model.occurences_sum());

        while (true) {
            if (b < HALF_INTERVAL) {
                bool_vector.push_back(0);
                for (auto i = decltype(licznik) {}; i < licznik; ++i) {
                    bool_vector.push_back(1);
                }
                licznik = 0;

            } else if (HALF_INTERVAL < a) {
                bool_vector.push_back(1);
                for (auto i = decltype(licznik) {}; i < licznik; ++i) {
                    bool_vector.push_back(0);
                }
                licznik = 0;

                a -= HALF_INTERVAL;
                b -= HALF_INTERVAL;
            } else if (QUARTER_INTERVAL <= a && b < 3 * QUARTER_INTERVAL) {
                a -= QUARTER_INTERVAL;
                b -= QUARTER_INTERVAL;
                ++licznik;
            } else {
                break;
            }
            a *= 2;
            b *= 2;
        }

        cache.update(symbol);
        if (cache.occurences_sum_ == BLOCK_SIZE) {
            model.update(cache);
            cache = adaptive::cache {};
        }
    }

    licznik += 1;
    if (a < QUARTER_INTERVAL) {
        bool_vector.push_back(0);
        for (auto i = decltype(licznik) {}; i < licznik; ++i) {
            bool_vector.push_back(1);
        }
    } else {
        bool_vector.push_back(1);
        for (auto i = decltype(licznik) {}; i < licznik; ++i) {
            bool_vector.push_back(0);
        }
    }

    output_vector = coding::misc::vector_cast(bool_vector);

    return output_vector;
}

inline auto decode(const std::vector<unsigned char>& input_vector, uint64_t amount_to_decode) -> std::vector<unsigned char>
{
    std::vector<unsigned char> output_vector {};
    output_vector.reserve(amount_to_decode);

    if (amount_to_decode == 0) {
        return output_vector;
    }

    adaptive::model model {};
    adaptive::cache cache {};

    std::vector<bool> bool_vector = coding::misc::vector_cast(input_vector);

    uint64_t a = 0;
    uint64_t b = WHOLE_INTERVAL;
    uint64_t z = 0;

    auto iter = bool_vector.begin();
    for (std::size_t i = 1; i <= BITS_PRECISION && iter < bool_vector.end(); ++i) {
        if (!(iter < bool_vector.end())) {
            break;
        }

        if (*iter) {
            z += uint64_t { 1 } << (BITS_PRECISION - i);
        }
        ++iter;
    }

    while (true) {
        std::size_t sym_left = 0;
        std::size_t sym_right = model.size() - 1;
        while (sym_left <= sym_right) {
            std::size_t symbol = sym_left + (sym_right - sym_left) / 2;

            uint64_t b_a = b - a;
            uint64_t sym_from = a + b_a * model.scale_from(symbol) / model.occurences_sum();
            uint64_t sym_to = a + b_a * model.scale_to(symbol) / model.occurences_sum();

            if (z < sym_from) {
                sym_right = symbol - 1;
            } else if (z >= sym_to) {
                sym_left = symbol + 1;
            } else {

                unsigned char decoded_symbol = static_cast<unsigned char>(symbol);
                output_vector.push_back(decoded_symbol);

                a = sym_from;
                b = sym_to;

                cache.update(symbol);
                if (cache.occurences_sum_ == BLOCK_SIZE) {
                    model.update(cache);
                    cache = adaptive::cache {};
                }

                if (output_vector.size() == amount_to_decode) {
                    return output_vector;
                } else {
                    break;
                }
            }
        }

        while (true) {
            if (b < HALF_INTERVAL) {

            } else if (HALF_INTERVAL < a) {

                a -= HALF_INTERVAL;
                b -= HALF_INTERVAL;
                z -= HALF_INTERVAL;

            } else if (QUARTER_INTERVAL <= a && b < 3 * QUARTER_INTERVAL) {

                a -= QUARTER_INTERVAL;
                b -= QUARTER_INTERVAL;
                z -= QUARTER_INTERVAL;
            } else {

                break;
            }
            a *= 2;
            b *= 2;
            z *= 2;

            if (iter < bool_vector.end()) {
                if (*iter) {
                    z += 1;
                }
                ++iter;
            }
        }
    }
}
}
