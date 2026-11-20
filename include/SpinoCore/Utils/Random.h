// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <array>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <random>
#include <type_traits>

namespace SpinoCore::Utils::Random {
    template <typename T>
    concept DistributionInteger =
        std::is_same_v<T, short> || std::is_same_v<T, int> || std::is_same_v<T, long> || std::is_same_v<T, long long> ||
        std::is_same_v<T, unsigned short> || std::is_same_v<T, unsigned int> ||
        std::is_same_v<T, unsigned long> || std::is_same_v<T, unsigned long long>;

    inline std::mt19937& GetEngine() noexcept {
        thread_local std::mt19937 rng = [] {
            std::random_device rd;

            std::array<std::mt19937::result_type, std::mt19937::state_size> seedData;
            for (auto& s : seedData) {
                s = rd();
            }

            std::seed_seq seq(seedData.begin(), seedData.end());
            return std::mt19937(seq);
        }();

        return rng;
    }

    inline void Seed(const std::uint32_t seed) noexcept {
        GetEngine().seed(seed);
    }

    template <std::floating_point T = float>
    [[nodiscard]] T GetFloatRange(const T min, const T max) noexcept {
        assert(min <= max && "Random::GetFloatRange - min must be less than or equal to max");
        std::uniform_real_distribution<T> dist(min, max);
        return dist(GetEngine());
    }

    template <DistributionInteger T = int>
    [[nodiscard]] T GetIntRange(const T min, const T max) noexcept {
        assert(min <= max && "Random::GetIntRange - min must be less than or equal to max");
        std::uniform_int_distribution<T> dist(min, max);
        return dist(GetEngine());
    }
}
