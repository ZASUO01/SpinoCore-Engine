// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

/// @brief Loading assets and writing files inside the Assets folder.
namespace SpinoCore::Files {
    /// @brief Kind of asset and the cache that receives it.
    enum class AssetType {
        /// Image file loaded into the texture cache.
        Texture,
        /// TrueType or OpenType font loaded into the font cache.
        Font,
        /// glTF or GLB model loaded into the model cache.
        Model,
        /// Audio file decoded into the sound cache.
        Sound,
        /// Not a valid asset type.
        None
    };

    /// @brief Queues asset loads and file writes and runs them a few per frame.
    ///
    /// Every path is relative to the Assets folder next to the executable. Loads are
    /// requested in Scene::OnStart, and Scene::OnSetup runs once IsReady returns true.
    class FileManager {
    public:
        virtual ~FileManager() = default;

        /// @brief Queues every asset listed in a JSON manifest.
        /// @param manifestPath Relative path of the manifest.
        /// @throws std::runtime_error When the manifest is missing or not valid; nothing is queued then.
        virtual void LoadFromManifest(const std::filesystem::path& manifestPath) = 0;
        /// @brief Queues one asset.
        /// @param type Kind of asset.
        /// @param id Name of the asset in its cache.
        /// @param path Relative path of the file.
        virtual void LoadFromSinglePath(AssetType type, std::string id, const std::filesystem::path& path) = 0;
        /// @brief Queues a write that replaces the file atomically.
        /// @param path Relative path of the file.
        /// @param data Bytes to write.
        virtual void WriteToSinglePath(const std::filesystem::path& path, std::vector<std::uint8_t> data) = 0;
        /// @param path Relative path.
        /// @return Whether a regular file exists at the path.
        [[nodiscard]] virtual bool FileExists(const std::filesystem::path& path) const = 0;
        /// @return Whether every queued load and write has finished.
        [[nodiscard]] virtual bool IsReady() const = 0;
        /// @return The names of the assets that failed to load; their caches return the default asset.
        [[nodiscard]] virtual const std::vector<std::string>& GetFailedLoads() const = 0;
    };
}
