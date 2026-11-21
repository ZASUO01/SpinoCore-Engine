// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "EngineFileManager.h"
#include <cstddef>
#include <exception>
#include <format>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_filesystem.h>
#include <nlohmann/json.hpp>
#include "FileHelpers.h"
#include "Graphics/Resources/Texture.h"
#include "SpinoCore/Logs/Logger.h"
#include "Utils/JSONHelpers.h"

namespace SpinoCore::Files {
    namespace {
        constexpr std::string_view AssetsDirectoryName{"Assets"};

        std::filesystem::path FindRootPath() {
            const char* basePath = SDL_GetBasePath();
            if (!basePath) {
                throw std::runtime_error(std::string("[FILE MANAGER] Failed to find the executable folder: ") + SDL_GetError());
            }
            return std::filesystem::path(std::u8string_view(reinterpret_cast<const char8_t*>(basePath))) / AssetsDirectoryName;
        }

        AssetType StringToAssetType(const std::string_view name) {
            if (name == "Texture") return AssetType::Texture;
            return AssetType::None;
        }

        std::filesystem::path PathFromUTF8(const std::string& text) {
            return {std::filesystem::path(std::u8string_view(reinterpret_cast<const char8_t*>(text.data()), text.size()))};
        }

        const std::string& RequireString(const Utils::JSON& asset, const std::string_view key, const std::size_t index,
                                         const std::filesystem::path& manifestPath) {
            const auto it = asset.find(key);
            if (it == asset.end() || !it->is_string() || it->get_ref<const std::string&>().empty()) {
                throw std::runtime_error(std::format("[FILE MANAGER] Asset {} in manifest {} needs a non-empty string \"{}\".",
                                                     index, manifestPath.string(), key));
            }
            return it->get_ref<const std::string&>();
        }
    }

    EngineFileManager::EngineFileManager(const FileManagerInput& input)
    :mRootPath(FindRootPath()),
    mTextureCache(input.textureCache)
    {
        if (!DirectoryExists(mRootPath)) throw std::runtime_error("[FILE MANAGER] Assets directory is missing: " + mRootPath.string());
    }

    void EngineFileManager::LoadFromManifest(const std::filesystem::path& manifestPath) {
        if (manifestPath.is_absolute()) {
            throw std::runtime_error(std::format("[FILE MANAGER] Manifest {} must be relative to the Assets folder.", manifestPath.string()));
        }

        const auto file = ReadFile(mRootPath / manifestPath);
        if (!file.has_value()) {
            throw std::runtime_error(std::format("[FILE MANAGER] Failed to load manifest file {}", manifestPath.string()));
        }

        const std::vector<std::string> requiredFields{"version", "assets"};
        const auto json = Utils::ParseJSON(file.value(), requiredFields);
        if (!json.has_value()) {
            throw std::runtime_error(std::format("[FILE MANAGER] Failed to parse json file {}", manifestPath.string()));
        }

        if (json->at("version") != "1.0") {
            throw std::runtime_error(std::format("[FILE MANAGER] Manifest {} has an invalid version; expected \"1.0\".", manifestPath.string()));
        }

        const auto& assets = json->at("assets");
        if (!assets.is_array()) {
            throw std::runtime_error(std::format("[FILE MANAGER] \"assets\" in manifest {} must be an array.", manifestPath.string()));
        }

        // Every entry is checked before any is queued, so a broken manifest loads nothing instead of half of it.
        std::vector<FileRequest> requests;
        requests.reserve(assets.size());
        for (std::size_t index = 0; index < assets.size(); ++index) {
            const auto& asset = assets[index];
            if (!asset.is_object()) {
                throw std::runtime_error(std::format("[FILE MANAGER] Asset {} in manifest {} must be an object.", index, manifestPath.string()));
            }

            const std::string& id = RequireString(asset, "name", index, manifestPath);
            const std::string& type = RequireString(asset, "type", index, manifestPath);
            const std::filesystem::path path = PathFromUTF8(RequireString(asset, "path", index, manifestPath));

            const auto assetType = StringToAssetType(type);
            if (assetType == AssetType::None) {
                throw std::runtime_error(std::format(R"([FILE MANAGER] Asset "{}" in manifest {} has an unknown type "{}".)",
                                                     id, manifestPath.string(), type));
            }
            if (path.is_absolute()) {
                throw std::runtime_error(std::format("[FILE MANAGER] Asset \"{}\" in manifest {} must have a path relative to the Assets folder.",
                                                     id, manifestPath.string()));
            }

            requests.push_back(FileRequest{.operation = Operation::Load, .type = assetType, .id = id, .path = path});
        }

        for (auto& request : requests) Enqueue(std::move(request));
    }

    void EngineFileManager::LoadFromSinglePath(const AssetType type, std::string id, const std::filesystem::path& path) {
        Enqueue(FileRequest{.operation = Operation::Load, .type = type, .id = std::move(id), .path = path, .data = {}});
    }

    void EngineFileManager::WriteToSinglePath(const std::filesystem::path& path, std::vector<std::uint8_t> data) {
        Enqueue(FileRequest{.operation = Operation::Write, .type = {}, .id = {}, .path = path, .data = std::move(data)});
    }

    bool EngineFileManager::FileExists(const std::filesystem::path& path) const {
        return !path.is_absolute() && Files::FileExists(mRootPath / path);
    }

    void EngineFileManager::ProcessRequests() {
        if (mRequestQueue.empty()) {
            mIsReady = true;
            return;
        }

        const auto start = std::chrono::steady_clock::now();

        do {
            Process(mRequestQueue.front());
            mRequestQueue.pop();
        } while (!mRequestQueue.empty() && std::chrono::steady_clock::now() - start < mWorkingPeriod);

        mIsReady = mRequestQueue.empty();
    }

    void EngineFileManager::Enqueue(FileRequest request) {
        if (request.path.is_absolute()) {
            Logs::Logger::Error("[FILES] \"{}\" must be relative to the Assets folder; the request was ignored.", request.path.string());
            return;
        }
        mRequestQueue.push(std::move(request));
        mIsReady = false;
    }

    void EngineFileManager::Process(const FileRequest& request) const {
        switch (request.operation) {
            case Operation::Load:
                switch (request.type) {
                    case AssetType::Texture: LoadTexture(request); break;
                    case AssetType::None: break;
                }
                break;
            case Operation::Write:
                static_cast<void>(WriteFile(mRootPath / request.path, request.data));
                break;
        }
    }

    void EngineFileManager::LoadTexture(const FileRequest& request) const {
        if (mTextureCache.Contains(request.id)) {
            Logs::Logger::Warn(R"([FILES] Texture "{}" is already cached; "{}" was not loaded.)", request.id, request.path.string());
            return;
        }

        try {
            mTextureCache.Add(request.id, std::make_unique<const Graphics::Resources::Texture>(mRootPath / request.path));
        } catch (const std::exception& e) {
            Logs::Logger::Error(R"([FILES] Failed to load texture "{}" from "{}": {})", request.id, request.path.string(), e.what());
        }
    }
}
