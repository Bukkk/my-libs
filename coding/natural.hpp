#include <array>
#include <cstdint>
#include <optional>
#include <stack>
#include <stdexcept>
#include <vector>

namespace coding::natural {
namespace {

    /**
     * @brief minimalna ilosc bitow potrzeba do zapisu liczby
     *
     * @param num liczba
     * @return uint32_t len minimalna taka num >> len == 0
     */
    inline auto bit_length(uint64_t num) -> uint32_t
    {
        // prawdopodobnie i tak kompilator zamieni w jedna instrukcje
        uint32_t len {};
        while (num != 0) {
            num >>= 1;
            ++len;
        }

        return len;
    }
}

struct elias_gamma {
    static constexpr bool padding = false;

    static auto encode(uint64_t num) -> std::vector<bool>
    {
        std::vector<bool> encoded;

        auto len = bit_length(num);
        for (auto i = decltype(len) {}; i < len - 1; ++i) {
            encoded.push_back(false);
        }

        for (auto i = decltype(len) {}; i < len; ++i) {
            encoded.push_back(((num >> (len - i - 1)) & 0x1) != 0);
        }

        return encoded;
    }

    static auto decode(
        std::vector<bool>::const_iterator& bits,
        const std::vector<bool>::const_iterator end)

        -> std::optional<uint64_t>
    {
        uint64_t decoded {};

        uint32_t zeros {};
        while (true) {

            if (!(bits < end)) {
                return {};
            }

            if (*bits) {
                break;
            }

            ++zeros;
            ++bits;
        }

        for (auto i = decltype(zeros) {}; i < zeros + 1; ++i) {
            if (!(bits < end)) {
                // tutaj moglby nawet zwracac blad bo jest wyraznie niedokonczony
                return {};
            }

            decoded <<= 1;
            decoded |= *bits ? 0x1 : 0x0;

            ++bits;
        }

        return { decoded };
    }
};

struct elias_delta {
    static constexpr bool padding = false;

    static auto encode(uint64_t num) -> std::vector<bool>
    {
        std::vector<bool> encoded;

        auto num_len = bit_length(num);
        auto len_len = bit_length(num_len);

        for (auto i = decltype(len_len) {}; i < len_len - 1; ++i) {
            encoded.push_back(false);
        }

        for (auto i = decltype(len_len) {}; i < len_len; ++i) {
            encoded.push_back(((num_len >> (len_len - i - 1)) & 0x1) != 0);
        }

        for (auto i = decltype(num_len) { 1 }; i < num_len; ++i) {
            encoded.push_back(((num >> (num_len - i - 1)) & 0x1) != 0);
        }

        return encoded;
    }

    static auto decode(
        std::vector<bool>::const_iterator& bits,
        const std::vector<bool>::const_iterator end)

        -> std::optional<uint64_t>
    {
        uint32_t zeros {};
        while (true) {

            if (!(bits < end)) {
                return {};
            }

            if (*bits) {
                break;
            }

            ++zeros;
            ++bits;
        }

        uint32_t len {};
        for (auto i = decltype(zeros) {}; i < zeros + 1; ++i) {
            if (!(bits < end)) {
                // tutaj moglby nawet zwracac blad bo jest wyraznie niedokonczony
                return {};
            }

            len <<= 1;
            len |= *bits ? 0x1 : 0x0;

            ++bits;
        }

        uint64_t decoded = 1;

        for (auto i = decltype(len) {}; i < len - 1; ++i) {
            if (!(bits < end)) {
                // tutaj moglby nawet zwracac blad bo jest wyraznie niedokonczony
                return {};
            }

            decoded <<= 1;
            decoded |= *bits ? 0x1 : 0x0;

            ++bits;
        }

        return decoded;
    }
};

struct elias_omega {
    static constexpr bool padding = true;

    static auto encode(uint64_t num) -> std::vector<bool>
    { // po prostu rozpisana rekursja aby nie allokowac rekursywnie
        std::vector<bool> encoded;

        if (num > 1) {
            // max 64biy min 1
            uint16_t num_len = bit_length(num); // 64 1

            // uint16_t x1 = num_len - 1; //63
            // uint16_t x1_len = bit_length(x1); //5

            // uint16_t x2 = x1_len - 1; //4
            // uint16_t x2_len = bit_length(x2); //3

            // uint16_t x3 = x2_len - 1; // 2
            // uint16_t x3_len = bit_length(x3); // 2

            std::array<uint16_t, 3> prefixes {};
            std::array<uint16_t, 3> lengths {};

            uint16_t x = num_len - 1;
            uint16_t i = 0;
            while (x != 1) {
                prefixes[i] = x;
                auto length = bit_length(x);
                lengths[i] = length;
                x = length - 1;
                ++i;
            }

            while (i > 0) {
                if (prefixes[i - 1] >= 2) {
                    auto write = prefixes[i - 1];
                    auto write_len = lengths[i - 1];
                    for (auto j = decltype(write_len) {}; j < write_len; ++j) {
                        encoded.push_back(((write >> (write_len - j - 1)) & 0x1) != 0);
                    }
                }

                --i;
            }

            for (auto i = decltype(num_len) {}; i < num_len; ++i) {
                encoded.push_back(((num >> (num_len - i - 1)) & 0x1) != 0);
            }
        }

        encoded.push_back(false);

        return encoded;
    }

    static auto decode(
        std::vector<bool>::const_iterator& bits,
        const std::vector<bool>::const_iterator end)

        -> std::optional<uint64_t>
    {
        uint64_t val = 1;
        while (true) {
            if (!(bits < end)) {
                return {};
            }

            if (!*bits) {
                ++bits;

                return { val };
                // } else {
                //     return {}; // zero jako pierwszy bit nic nie koduje
                // }
            }

            uint64_t new_val {};
            for (uint64_t i = 0; i < val + 1; ++i) {
                if (!(bits < end)) {
                    // tutaj moglby nawet zwracac blad bo jest wyraznie niedokonczony
                    return {};
                }

                new_val <<= 1;
                new_val |= *bits ? 0x1 : 0x0;

                ++bits;
            }

            val = new_val;
        }
    }
};

namespace {
    template <typename T>
    constexpr auto fib_len()
    {
        T fib1 = 1;
        T fib2 = 2;

        auto i = 2;
        while (true) {
            T fib = fib2 + fib1;

            if (fib < fib2) {
                return i;
            }

            ++i;
            fib1 = fib2;
            fib2 = fib;
        }
    }

    template <typename T, uint16_t N>
    constexpr auto fib_lookup() -> std::array<T, N>
    {
        std::array<T, N> table {};
        table[0] = 1;
        table[1] = 2;

        for (auto i = 2; i < N; ++i) {
            table[i] = table[i - 1] + table[i - 2];
        }

        return table;
    }
}

struct fibonacci {
    static constexpr bool padding = false;

private:
    constexpr static auto fibonacci_lookup_table
        = fib_lookup<uint64_t, fib_len<uint64_t>()>();

    static auto find_fibonacci_index(uint64_t num) -> uint16_t
    {
        // dla 64bit nawet sie nie wygeneruje
        constexpr static auto fiblen = fib_len<uint64_t>();

        uint16_t index = 0;
        while (index < 64 && index < fiblen) {
            if (fibonacci_lookup_table[index + 1] <= num) {
                index = index + 1;
            } else {
                break;
            }
        }

        return index;
    }

public:
    static auto encode(uint64_t num) -> std::vector<bool>
    {
        std::vector<bool> encoded;

        uint64_t to_encode {};

        auto max_index = 0;
        while (num > 0) {
            auto index = find_fibonacci_index(num);
            if (index > 63) {
                throw std::runtime_error { "za duza liczba" };
            }

            max_index = max_index >= index ? max_index : index;
            to_encode |= decltype(to_encode) { 1 } << index;
            num -= fibonacci_lookup_table[index];
        }

        for (auto i = decltype(max_index) {}; i < max_index + 1; ++i) {
            bool v = ((to_encode >> i) & 0x1) != 0;
            encoded.push_back(v);
        }

        encoded.push_back(true);

        return encoded;
    }

    static auto decode(
        std::vector<bool>::const_iterator& bits,
        const std::vector<bool>::const_iterator end)

        -> std::optional<uint64_t>
    {
        uint64_t decoded = 0;

        bool last_was_true = false;
        uint16_t index = 0;
        while (true) {
            if (!(bits < end)) {
                return {};
            }
            bool bit = *bits;
            ++bits;

            if (bit) {
                if (last_was_true) {
                    return decoded;
                }

                decoded += fibonacci_lookup_table[index];
                last_was_true = true;
            } else {
                last_was_true = false;
            }

            ++index;
        }
    }
};

}