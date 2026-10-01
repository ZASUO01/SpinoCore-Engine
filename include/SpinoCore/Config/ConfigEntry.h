// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <string>
#include <variant>
#include <cstdint>

namespace SpinoCore::Config::Entry {
    using ConfigValue = std::variant<unsigned int, int, float, bool, std::string>;

    // FNV hash algorithm
    // NOLINTNEXTLINE
    constexpr uint32_t HashString(const std::string_view str) {
        uint32_t hash = 2166136261u;
        for (const char c : str) {
            hash ^= static_cast<uint32_t>(c);
            hash *= 16777619u;
        }
        return hash;
    }

    struct ConfigEntry {
        ConfigValue value;
        bool isReadOnly{false};
    };


    struct ConfigKey {
        uint32_t hash;


        template <std::size_t N>
        // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
        consteval ConfigKey(const char (&str)[N])
            : hash(HashString(std::string_view(str, N - 1))) {}

        explicit constexpr  ConfigKey(const uint32_t hash) : hash(hash) {}
    };
}