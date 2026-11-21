// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <nlohmann/json_fwd.hpp>

namespace SpinoCore::Utils {
    using JSON = nlohmann::json;
    [[nodiscard]] std::optional<JSON> ParseJSON(const std::vector<std::uint8_t>& data, const std::vector<std::string>& requiredFields = {});
}
