// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <vector>

namespace SpinoCore::Files {
    [[nodiscard]] bool DirectoryExists(const std::filesystem::path& path);
    [[nodiscard]] bool FileExists(const std::filesystem::path& path);
    [[nodiscard]] std::optional<std::vector<std::uint8_t>> ReadFile(const std::filesystem::path& filePath);
    [[nodiscard]] bool WriteFile(const std::filesystem::path& filePath, std::span<const std::uint8_t> data);
}
