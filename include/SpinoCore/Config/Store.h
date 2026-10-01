// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include <unordered_map>
#include "ConfigEntry.h"

namespace SpinoCore::Config {
    class Store {
    public:
        Store() = default;
        ~Store() = default;

        Store (const Store&) = delete;
        Store& operator= (const Store&) = delete;
        Store (Store&&) = delete;
        Store& operator= (Store&&) = delete;

        void Set(const Entry::ConfigKey& key, const Entry::ConfigValue& value, const bool readOnly = false) {
            if (const auto it = mConfigEntries.find(key.hash); it != mConfigEntries.end() && it->second.isReadOnly) {
                return;
            }

            mConfigEntries[key.hash] = Entry::ConfigEntry{.value = value, .isReadOnly = readOnly};
        }

        template <typename T>
        [[nodiscard]] T Get(const Entry::ConfigKey& key, const T& defaultValue) const {
            if (const auto it = mConfigEntries.find(key.hash); it != mConfigEntries.end()) {
                if (const T* value = std::get_if<T>(&it->second.value)) {
                    return *value;
                }
            }

            return defaultValue;
        }

        [[nodiscard]] std::string Get(const Entry::ConfigKey& key, const char* defaultValue) const {
            return Get<std::string>(key, std::string(defaultValue));
        }

    private:
        std::unordered_map<uint32_t, Entry::ConfigEntry> mConfigEntries;
    };
}