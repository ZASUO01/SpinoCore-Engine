// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "JSONUtils.h"
#include "SpinoCore/Logs/Logger.h"
#include "nlohmann/json.hpp"

namespace SpinoCore::Utils::JSON {
    std::optional<json> ParseJSONFromBytes(
        const std::vector<uint8_t>& fileData,
        const std::vector<std::string>& requiredFields
    ) {
        if (fileData.empty()) {
            Logs::Logger::Error("[JSON UTILS] The data buffer is empty.");
            return std::nullopt;
        }

        nlohmann::json parsed = nlohmann::json::parse(
            fileData.begin(),
            fileData.end(),
            nullptr,
            false
        );

        if (parsed.is_discarded()) {
            Logs::Logger::Error("[JSON UTILS] Invalid or corrupted JSON file.");
            return std::nullopt;
        }

        for (const auto& path : requiredFields) {
            if (path.empty()) continue;

            const nlohmann::json* currentNode = &parsed;
            std::string_view remainingPath = path;
            size_t depth = 0;

            while (!remainingPath.empty()) {
                if (constexpr int MAX_DEPTH = 5; depth >= MAX_DEPTH) {
                    Logs::Logger::Error("[JSON UTILS] The path '{}' exceeds the maximum allowed search depth.", path);
                    return std::nullopt;
                }

                const size_t pos = remainingPath.find('/');
                const std::string_view key = remainingPath.substr(0, pos);

                if (!currentNode->is_object() || !currentNode->contains(key)) {
                    Logs::Logger::Error("[JSON UTILS] The required field '{}' is missing in the path '{}'.", key, path);
                    return std::nullopt;
                }

                currentNode = &currentNode->at(std::string(key));
                depth++;

                if (pos == std::string_view::npos) break;

                remainingPath.remove_prefix(pos + 1);
            }
        }

        return parsed;
    }
}
