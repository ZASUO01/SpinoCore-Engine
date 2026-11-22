// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <array>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <random>
#include <type_traits>

/// @brief Random numbers from a per-thread generator.
namespace SpinoCore::Utils::Random {
    /// @brief Integer types accepted by std::uniform_int_distribution.
    template <typename T>
    concept DistributionInteger =
        std::is_same_v<T, short> || std::is_same_v<T, int> || std::is_same_v<T, long> || std::is_same_v<T, long long> ||
        std::is_same_v<T, unsigned short> || std::is_same_v<T, unsigned int> ||
        std::is_same_v<T, unsigned long> || std::is_same_v<T, unsigned long long>;

    /// @return The generator of the calling thread, seeded from std::random_device.
    inline std::mt19937& GetEngine() noexcept {
        thread_local std::mt19937 rng = [] {
            std::random_device rd;

            std::array<std::mt19937::result_type, std::mt19937::state_size> seedData{};
            for (auto& s : seedData) {
                s = rd();
            }

            std::seed_seq seq(seedData.begin(), seedData.end());
            return std::mt19937(seq);
        }();

        return rng;
    }

    /// @brief Reseeds the generator of the calling thread, which makes the sequence repeatable.
    /// @param seed New seed.
    inline void Seed(const std::uint32_t seed) noexcept {
        GetEngine().seed(seed);
    }

    /// @brief Draws a real number.
    /// @tparam T Floating point type.
    /// @param min Lowest value.
    /// @param max Highest value; it must not be less than min.
    /// @return A value in [min, max).
    template <std::floating_point T = float>
    [[nodiscard]] T GetFloatRange(const T min, const T max) noexcept {
        assert(min <= max && "Random::GetFloatRange - min must be less than or equal to max");
        std::uniform_real_distribution<T> dist(min, max);
        return dist(GetEngine());
    }

    /// @brief Draws an integer.
    /// @tparam T Integer type.
    /// @param min Lowest value.
    /// @param max Highest value; it must not be less than min.
    /// @return A value in [min, max].
    template <DistributionInteger T = int>
    [[nodiscard]] T GetIntRange(const T min, const T max) noexcept {
        assert(min <= max && "Random::GetIntRange - min must be less than or equal to max");
        std::uniform_int_distribution<T> dist(min, max);
        return dist(GetEngine());
    }
}
