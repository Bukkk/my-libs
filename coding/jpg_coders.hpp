#pragma once

#include "moje_tga.hpp"

#include <cstddef>
#include <cstdint>
#include <tuple>

namespace coding {

namespace jpg_predictors {
    struct predictor_1 {
        static auto predict(tga::accessor_MONO const& acc, uint32_t x, uint32_t y) -> uint8_t
        {
            uint8_t w = acc.check_range(x - 1, y) ? acc[acc.nth(x - 1, y)] : 0;
            // uint8_t n = acc.check_range(x, y + 1) ? acc[acc.nth(x, y + 1)] : 0;
            // uint8_t nw = acc.check_range(x - 1, y + 1) ? acc[acc.nth(x - 1, y + 1)] : 0;

            return w;
        }
    };

    struct predictor_2 {
        static auto predict(tga::accessor_MONO const& acc, uint32_t x, uint32_t y) -> uint8_t
        {
            // uint8_t w = acc.check_range(x - 1, y) ? acc[acc.nth(x - 1, y)] : 0;
            uint8_t n = acc.check_range(x, y + 1) ? acc[acc.nth(x, y + 1)] : 0;
            // uint8_t nw = acc.check_range(x - 1, y + 1) ? acc[acc.nth(x - 1, y + 1)] : 0;

            return n;
        }
    };

    struct predictor_3 {
        static auto predict(tga::accessor_MONO const& acc, uint32_t x, uint32_t y) -> uint8_t
        {
            // uint8_t w = acc.check_range(x - 1, y) ? acc[acc.nth(x - 1, y)] : 0;
            // uint8_t n = acc.check_range(x, y + 1) ? acc[acc.nth(x, y + 1)] : 0;
            uint8_t nw = acc.check_range(x - 1, y + 1) ? acc[acc.nth(x - 1, y + 1)] : 0;

            return nw;
        }
    };

    struct predictor_4 {
        static auto predict(tga::accessor_MONO const& acc, uint32_t x, uint32_t y) -> uint8_t
        {
            uint8_t w = acc.check_range(x - 1, y) ? acc[acc.nth(x - 1, y)] : 0;
            uint8_t n = acc.check_range(x, y + 1) ? acc[acc.nth(x, y + 1)] : 0;
            uint8_t nw = acc.check_range(x - 1, y + 1) ? acc[acc.nth(x - 1, y + 1)] : 0;

            return n + w - nw;
        }
    };

    struct predictor_5 {
        static auto predict(tga::accessor_MONO const& acc, uint32_t x, uint32_t y) -> uint8_t
        {
            uint8_t w = acc.check_range(x - 1, y) ? acc[acc.nth(x - 1, y)] : 0;
            uint8_t n = acc.check_range(x, y + 1) ? acc[acc.nth(x, y + 1)] : 0;
            uint8_t nw = acc.check_range(x - 1, y + 1) ? acc[acc.nth(x - 1, y + 1)] : 0;

            return n + (w - nw) / 2;
        }
    };

    struct predictor_6 {
        static auto predict(tga::accessor_MONO const& acc, uint32_t x, uint32_t y) -> uint8_t
        {
            uint8_t w = acc.check_range(x - 1, y) ? acc[acc.nth(x - 1, y)] : 0;
            uint8_t n = acc.check_range(x, y + 1) ? acc[acc.nth(x, y + 1)] : 0;
            uint8_t nw = acc.check_range(x - 1, y + 1) ? acc[acc.nth(x - 1, y + 1)] : 0;

            return w + (n - nw) / 2;
        }
    };

    struct predictor_7 {
        static auto predict(tga::accessor_MONO const& acc, uint32_t x, uint32_t y) -> uint8_t
        {
            uint8_t w = acc.check_range(x - 1, y) ? acc[acc.nth(x - 1, y)] : 0;
            uint8_t n = acc.check_range(x, y + 1) ? acc[acc.nth(x, y + 1)] : 0;
            // uint8_t nw = acc.check_range(x - 1, y + 1) ? acc[acc.nth(x - 1, y + 1)] : 0;

            return (n + w) / 2;
        }
    };

    struct predictor_new {
        static auto predict(tga::accessor_MONO const& acc, uint32_t x, uint32_t y) -> uint8_t
        {
            uint8_t w = acc.check_range(x - 1, y) ? acc[acc.nth(x - 1, y)] : 0;
            uint8_t n = acc.check_range(x, y + 1) ? acc[acc.nth(x, y + 1)] : 0;
            uint8_t nw = acc.check_range(x - 1, y + 1) ? acc[acc.nth(x - 1, y + 1)] : 0;

            if (nw >= std::max(w, n)) {
                return std::min(w, n);
            } else if (nw <= std::min(w, n)) {
                return std::max(w, n);
            }

            return w + n - nw;
        }
    };

}

namespace differential_coding {

    template <typename Predictor>
    auto encode(tga::accessor_MONO const& real_data) -> std::vector<uint8_t>
    {
        std::vector<uint8_t> vals;
        vals.resize(real_data._image.size());

        tga::accessor_MONO diff_data = {vals, real_data._width, real_data._height};

        for (size_t y {}; y < real_data._height; ++y) {
            for (size_t x {}; x < real_data._width; ++x) {
                auto nth = real_data.nth(x, y);
                uint8_t predicted = Predictor::predict(real_data, x, y);
                uint8_t real = real_data[nth];

                uint8_t diff = real - predicted;
                diff_data[nth] = diff;
            }
        }

        return vals;
    }

    template <typename Predictor>
    auto decode(tga::accessor_MONO const& diff_data) -> std::vector<uint8_t>
    {
        std::vector<uint8_t> vals;
        vals.resize(diff_data._image.size());

        tga::accessor_MONO real_data = {vals, diff_data._width, diff_data._height};

        for (size_t y_ {}; y_ < real_data._height; ++y_) {
            for (size_t x {}; x < real_data._width; ++x) {
                auto y = real_data._height - y_ - 1;
                
                auto nth = real_data.nth(x, y);
                uint8_t predicted = Predictor::predict(real_data, x, y);
                uint8_t diff = diff_data[nth];

                uint8_t real = predicted + diff;
                real_data[nth] = real;
            }
        }

        return vals;
    }

}

}