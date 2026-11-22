// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace SpinoCore::Files {
    enum class AssetType {
        Texture,
        Font,
        None
    };

    class FileManager {
    public:
        virtual ~FileManager() = default;

        virtual void LoadFromManifest(const std::filesystem::path& manifestPath) = 0;
        virtual void LoadFromSinglePath(AssetType type, std::string id, const std::filesystem::path& path) = 0;
        virtual void WriteToSinglePath(const std::filesystem::path& path, std::vector<std::uint8_t> data) = 0;
        [[nodiscard]] virtual bool FileExists(const std::filesystem::path& path) const = 0;
        [[nodiscard]] virtual bool IsReady() const = 0;
        [[nodiscard]] virtual const std::vector<std::string>& GetFailedLoads() const = 0;
    };
}
