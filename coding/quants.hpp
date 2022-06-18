#pragma once

#include "tga.hpp"
#include "quantization.hpp"
#include "graphics_statistics.hpp"

#include <cstdint>
#include <functional>
#include <optional>

struct quants {
    uint32_t r_quant {};
    uint32_t g_quant {};
    uint32_t b_quant {};
};

inline auto operator<<(std::ostream& o, quants const& q) -> std::ostream&
{
    // clang-format off
    o << "{"
        << "r_bits=" << q.r_quant << ","
        << "g_bits=" << q.g_quant << ","
        << "b_bits=" << q.b_quant
        << "}";
    // clang-format on

    return o;
}

using quant_chooser = std::function<quants(std::vector<uint8_t> const&)>;

namespace quant_choosers {

struct mse {
    uint32_t bits_ { 24 };

    mse(uint32_t bits)
        : bits_ { bits }
    {
    }
    auto operator()(std::vector<uint8_t> const& rgb_vals) -> quants
    {

        auto [r_vals, g_vals, b_vals] = tga::split_channels(rgb_vals);

        quants best = { 0, 0, 0 };
        auto mse_best = std::optional<double> {};
        for (uint32_t q_r {}; q_r <= bits_; ++q_r) {
            for (uint32_t q_g {}; q_g <= bits_ - q_r; ++q_g) {
                uint32_t q_b = bits_ - q_r - q_g;

                std::vector<uint8_t> r_vals_quantized = coding::uniform_quantization(r_vals, q_r);
                std::vector<uint8_t> g_vals_quantized = coding::uniform_quantization(g_vals, q_g);
                std::vector<uint8_t> b_vals_quantized = coding::uniform_quantization(b_vals, q_b);

                // std::vector<uint8_t> rgb_vals_quantized = tga::join_channels(r_vals_quantized, g_vals_quantized, b_vals_quantized);
                // double mse = coding::statistics::mse(rgb_vals, rgb_vals_quantized);

                double mse_r = coding::statistics::mse(r_vals, r_vals_quantized);
                double mse_g = coding::statistics::mse(g_vals, g_vals_quantized);
                double mse_b = coding::statistics::mse(b_vals, b_vals_quantized);
                double mse = std::max(std::max(mse_r, mse_g), mse_b);

                if (!mse_best || (mse_best && *mse_best > mse)) {
                    mse_best = mse;
                    best = { q_r, q_g, q_b };
                }
            }
        }

        return best;
    }
};

struct snr {
    uint32_t bits_ { 24 };

    snr(uint32_t bits)
        : bits_ { bits }
    {
    }
    auto operator()(std::vector<uint8_t> const& rgb_vals) -> quants
    {

        auto [r_vals, g_vals, b_vals] = tga::split_channels(rgb_vals);

        quants best = { 0, 0, 0 };
        auto snr_best = std::optional<double> {};
        for (uint32_t q_r {}; q_r <= bits_; ++q_r) {
            for (uint32_t q_g {}; q_g <= bits_ - q_r; ++q_g) {
                uint32_t q_b = bits_ - q_r - q_g;

                std::vector<uint8_t> r_vals_quantized = coding::uniform_quantization(r_vals, q_r);
                std::vector<uint8_t> g_vals_quantized = coding::uniform_quantization(g_vals, q_g);
                std::vector<uint8_t> b_vals_quantized = coding::uniform_quantization(b_vals, q_b);

                // std::vector<uint8_t> rgb_vals_quantized = tga::join_channels(r_vals_quantized, g_vals_quantized, b_vals_quantized);
                // double mse = coding::statistics::mse(rgb_vals, rgb_vals_quantized);
                // double snr = coding::statistics::snr(rgb_vals, mse);

                double mse_r = coding::statistics::mse(r_vals, r_vals_quantized);
                double mse_g = coding::statistics::mse(g_vals, g_vals_quantized);
                double mse_b = coding::statistics::mse(b_vals, b_vals_quantized);
                double snr_r = coding::statistics::snr(r_vals, mse_r);
                double snr_g = coding::statistics::snr(g_vals, mse_g);
                double snr_b = coding::statistics::snr(b_vals, mse_b);
                double snr = std::min(std::min(snr_r, snr_g), snr_b);

                if (!snr_best || (snr_best && *snr_best < snr)) {
                    snr_best = snr;
                    best = { q_r, q_g, q_b };
                }
            }
        }

        return best;
    }
};

}