// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <queue>
#include <string>
#include <vector>
#include "Assets/AssetCache.h"
#include "SpinoCore/Files/FileManager.h"

namespace SpinoCore::Graphics::Resources {
    class Texture;
}

namespace SpinoCore::Files {
    struct FileManagerInput final{
        Assets::AssetCache<Graphics::Resources::Texture>& textureCache;
    };

    class EngineFileManager final : public FileManager{
    public:
        explicit EngineFileManager(const FileManagerInput& input);
        ~EngineFileManager() override = default;

        EngineFileManager(const EngineFileManager&) = delete;
        EngineFileManager& operator=(const EngineFileManager&) = delete;
        EngineFileManager(EngineFileManager&&) = delete;
        EngineFileManager& operator=(EngineFileManager&&) = delete;

        void LoadFromManifest(const std::filesystem::path& manifestPath) override;
        void LoadFromSinglePath(AssetType type, std::string id, const std::filesystem::path& path) override;
        void WriteToSinglePath(const std::filesystem::path& path, std::vector<std::uint8_t> data) override;
        [[nodiscard]] bool FileExists(const std::filesystem::path& path) const override;

        void ProcessRequests();
        [[nodiscard]] bool IsReady() const { return mIsReady; }
        [[nodiscard]] const std::filesystem::path& GetRootPath() const noexcept { return mRootPath; }

    private:
        using Duration = std::chrono::nanoseconds;

        enum class Operation {
            Load,
            Write,
        };

        struct FileRequest {
            Operation operation;
            AssetType type;
            std::string id;
            std::filesystem::path path;
            std::vector<std::uint8_t> data{};
        };

        void Enqueue(FileRequest request);
        void Process(const FileRequest& request) const;
        void LoadTexture(const FileRequest& request) const;

        std::queue<FileRequest> mRequestQueue;

        const std::filesystem::path mRootPath;
        const Duration mWorkingPeriod{4'000'000};
        bool mIsReady{true};

        Assets::AssetCache<Graphics::Resources::Texture>& mTextureCache;
    };
}
