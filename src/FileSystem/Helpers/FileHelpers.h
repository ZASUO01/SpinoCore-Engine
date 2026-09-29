// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <filesystem>
#include <limits>
#include <optional>
#include <vector>

namespace SpinoCore::FileSystem::Helpers {
    static constexpr uint64_t IGNORE_OFFSET = std::numeric_limits<uint64_t>::max();
    static constexpr uint64_t READ_ALL = 0;

    [[nodiscard]] std::optional<std::vector<uint8_t>> ReadFile(
        const std::filesystem::path& path,
        uint64_t offset = 0,
        uint64_t sizeToRead = READ_ALL
    );

    [[nodiscard]] bool WriteToFile(
            const std::filesystem::path& path,
            const std::vector<uint8_t>& data,
            uint64_t offset = IGNORE_OFFSET
    );
}