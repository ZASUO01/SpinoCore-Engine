// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "JSONHelpers.h"
#include <cstddef>
#include <string_view>
#include <nlohmann/json.hpp>
#include "SpinoCore/Logs/Logger.h"

namespace SpinoCore::Utils {
    namespace {
        constexpr char FieldSeparator = '/';

        [[nodiscard]] bool HasField(const JSON& root, const std::string_view field) {
            const JSON* current = &root;
            std::size_t start = 0;

            while (true) {
                const std::size_t end = field.find(FieldSeparator, start);
                const std::string_view key = field.substr(start, end == std::string_view::npos ? std::string_view::npos : end - start);

                if (key.empty() || !current->is_object()) return false;

                const auto it = current->find(key);
                if (it == current->end()) return false;
                current = &*it;

                if (end == std::string_view::npos) return true;
                start = end + 1;
            }
        }
    }

    std::optional<JSON> ParseJSON(const std::vector<std::uint8_t>& data, const std::vector<std::string>& requiredFields) {
        JSON json;
        try {
            json = JSON::parse(data.begin(), data.end());
        } catch (const JSON::exception& e) {
            Logs::Logger::Error("[JSON] Failed to parse: {}", e.what());
            return std::nullopt;
        }

        bool hasAllFields = true;
        for (const std::string& field : requiredFields) {
            if (!HasField(json, field)) {
                Logs::Logger::Error("[JSON] Missing required field \"{}\"", field);
                hasAllFields = false;
            }
        }
        if (!hasAllFields) return std::nullopt;

        return json;
    }
}
