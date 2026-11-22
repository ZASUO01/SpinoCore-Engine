// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "EngineFileManager.h"
#include <algorithm>
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
#include "Graphics/Resources/Font.h"
#include "Graphics/Resources/Model.h"
#include "Graphics/Resources/ModelData.h"
#include "Graphics/Resources/Texture.h"
#include "SDL/Helpers/Helpers.h"
#include "Sound/Resources/AudioClip.h"
#include "SpinoCore/Math/Vector.h"
#include "SpinoCore/Logs/Logger.h"
#include "Utils/JSONHelpers.h"
#include "Utils/ModelHelpers.h"

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
            if (name == "Font") return AssetType::Font;
            if (name == "Model") return AssetType::Model;
            if (name == "Sound") return AssetType::Sound;
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

        std::filesystem::path OptionalPath(const Utils::JSON& asset, const std::string_view key, const std::string& id,
                                           const std::filesystem::path& manifestPath) {
            const auto it = asset.find(key);
            if (it == asset.end()) return {};
            if (!it->is_string() || it->get_ref<const std::string&>().empty()) {
                throw std::runtime_error(std::format(R"([FILE MANAGER] "{}" of asset "{}" in manifest {} must be a non-empty string.)",
                                                     key, id, manifestPath.string()));
            }

            std::filesystem::path path = PathFromUTF8(it->get_ref<const std::string&>());
            if (path.is_absolute()) {
                throw std::runtime_error(std::format(R"([FILE MANAGER] "{}" of asset "{}" in manifest {} must be relative to the Assets folder.)",
                                                     key, id, manifestPath.string()));
            }
            return path;
        }

        struct ManifestEntry final {
            AssetType type{AssetType::None};
            std::string id;
            std::filesystem::path path;
            std::filesystem::path texturePath;
            std::filesystem::path materialPath;
        };

        ManifestEntry ParseManifestEntry(const Utils::JSON& asset, const std::size_t index, const std::filesystem::path& manifestPath) {
            if (!asset.is_object()) {
                throw std::runtime_error(std::format("[FILE MANAGER] Asset {} in manifest {} must be an object.", index, manifestPath.string()));
            }

            ManifestEntry entry;
            entry.id = RequireString(asset, "name", index, manifestPath);
            const std::string& type = RequireString(asset, "type", index, manifestPath);
            entry.path = PathFromUTF8(RequireString(asset, "path", index, manifestPath));

            entry.type = StringToAssetType(type);
            if (entry.type == AssetType::None) {
                throw std::runtime_error(std::format(R"([FILE MANAGER] Asset "{}" in manifest {} has an unknown type "{}".)",
                                                     entry.id, manifestPath.string(), type));
            }
            if (entry.path.is_absolute()) {
                throw std::runtime_error(std::format("[FILE MANAGER] Asset \"{}\" in manifest {} must have a path relative to the Assets folder.",
                                                     entry.id, manifestPath.string()));
            }

            if (entry.type == AssetType::Model) {
                entry.texturePath = OptionalPath(asset, "texture", entry.id, manifestPath);
                entry.materialPath = OptionalPath(asset, "material", entry.id, manifestPath);
            }
            return entry;
        }

        struct MaterialOverride final {
            std::optional<Math::Vector4f> baseColor{};
            std::filesystem::path texturePath{};
        };

        MaterialOverride ReadMaterialFile(const std::filesystem::path& rootPath, const std::filesystem::path& materialPath) {
            const auto file = ReadFile(rootPath / materialPath);
            if (!file.has_value()) throw std::runtime_error(std::format("could not read the material file {}", materialPath.string()));

            const auto json = Utils::ParseJSON(file.value(), {"version"});
            if (!json.has_value() || !json->is_object() || json->at("version") != "1.0") {
                throw std::runtime_error(std::format(R"(material file {} must be a JSON object with "version": "1.0")", materialPath.string()));
            }

            MaterialOverride material{};
            if (const auto color = json->find("baseColor"); color != json->end()) {
                if (!color->is_array() || color->size() != 4 || !std::ranges::all_of(*color, [](const auto& value) { return value.is_number(); })) {
                    throw std::runtime_error(std::format(R"("baseColor" in material file {} must be an array of 4 numbers)", materialPath.string()));
                }
                material.baseColor = Math::Vector4f{(*color)[0].get<float>(), (*color)[1].get<float>(), (*color)[2].get<float>(), (*color)[3].get<float>()};
            }
            material.texturePath = OptionalPath(*json, "texture", materialPath.string(), materialPath);
            return material;
        }
    }

    EngineFileManager::EngineFileManager(const FileManagerInput& input)
    :mRootPath(FindRootPath())
    ,mTextureCache(input.textureCache)
    ,mFontCache(input.fontCache)
    ,mModelCache(input.modelCache)
    ,mSoundCache(input.soundCache)
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

        std::vector<FileRequest> requests;
        requests.reserve(assets.size());
        for (std::size_t index = 0; index < assets.size(); ++index) {
            ManifestEntry entry = ParseManifestEntry(assets[index], index, manifestPath);
            requests.push_back(FileRequest{.operation = Operation::Load, .type = entry.type, .id = std::move(entry.id),
                                           .path = std::move(entry.path), .data = {}, .texturePath = std::move(entry.texturePath),
                                           .materialPath = std::move(entry.materialPath)});
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
        if (mIsReady) mFailedLoads.clear();
        mRequestQueue.push(std::move(request));
        mIsReady = false;
    }

    void EngineFileManager::Process(const FileRequest& request) {
        switch (request.operation) {
            case Operation::Load:
                switch (request.type) {
                    case AssetType::Texture: LoadTexture(request); break;
                    case AssetType::Font: LoadFont(request); break;
                    case AssetType::Model: LoadModel(request); break;
                    case AssetType::Sound: LoadSound(request); break;
                    default:
                        break;
                }
                break;
            case Operation::Write:
                static_cast<void>(WriteFile(mRootPath / request.path, request.data));
                break;
        }
    }

    void EngineFileManager::LoadTexture(const FileRequest& request) {
        if (mTextureCache.Contains(request.id)) {
            Logs::Logger::Warn(R"([FILES] Texture "{}" is already cached; "{}" was not loaded.)", request.id, request.path.string());
            return;
        }

        try {
            mTextureCache.Add(request.id, std::make_unique<const Graphics::Resources::Texture>(mRootPath / request.path));
        } catch (const std::exception& e) {
            Logs::Logger::Error(R"([FILES] Failed to load texture "{}" from "{}": {})", request.id, request.path.string(), e.what());
            mFailedLoads.push_back(request.id);
        }
    }

    void EngineFileManager::LoadFont(const FileRequest &request) {
        if (mFontCache.Contains(request.id)) {
            Logs::Logger::Warn(R"([FILES] Font "{}" is already cached; "{}" was not loaded.)", request.id, request.path.string());
            return;
        }

        try {
            mFontCache.Add(request.id, std::make_unique<const Graphics::Resources::Font>(mRootPath / request.path));
        } catch (const std::exception& e) {
            Logs::Logger::Error(R"([FILES] Failed to load font "{}" from "{}": {})", request.id, request.path.string(), e.what());
            mFailedLoads.push_back(request.id);
        }
    }

    void EngineFileManager::LoadModel(const FileRequest& request) {
        if (mModelCache.Contains(request.id)) {
            Logs::Logger::Warn(R"([FILES] Model "{}" is already cached; "{}" was not loaded.)", request.id, request.path.string());
            return;
        }

        try {
            const auto file = ReadFile(mRootPath / request.path);
            if (!file.has_value()) throw std::runtime_error("could not read the file");

            const Graphics::Resources::ModelData data = Utils::ParseModel(file.value(), mRootPath / request.path);
            const MaterialOverride material = request.materialPath.empty() ? MaterialOverride{} : ReadMaterialFile(mRootPath, request.materialPath);
            const std::filesystem::path& sharedTexture = request.texturePath.empty() ? material.texturePath : request.texturePath;

            std::vector<Graphics::Resources::Model::Material> materials;
            materials.reserve(data.materials.size());
            for (std::size_t index = 0; index < data.materials.size(); ++index) {
                const auto& source = data.materials[index];
                Graphics::Resources::Model::Material resolved{.baseColor = material.baseColor.value_or(source.baseColor), .textureId = {}};

                if (!sharedTexture.empty()) {
                    resolved.textureId = LoadTextureFile(sharedTexture);
                } else if (source.texturePath.has_value()) {
                    resolved.textureId = LoadTextureFile(request.path.parent_path() / source.texturePath.value());
                } else if (!source.embeddedTexture.empty()) {
                    resolved.textureId = std::format("{}#{}", request.id, index);
                    if (!mTextureCache.Contains(resolved.textureId)) {
                        const auto surface = SDL::Helpers::CreateFromImage(source.embeddedTexture);
                        if (!surface) throw std::runtime_error(std::format("could not decode the texture of material {}", index));
                        mTextureCache.Add(resolved.textureId, std::make_unique<const Graphics::Resources::Texture>(surface.get()));
                    }
                }
                materials.push_back(std::move(resolved));
            }

            mModelCache.Add(request.id, std::make_unique<const Graphics::Resources::Model>(data, std::move(materials)));
        } catch (const std::exception& e) {
            Logs::Logger::Error(R"([FILES] Failed to load model "{}" from "{}": {})", request.id, request.path.string(), e.what());
            mFailedLoads.push_back(request.id);
        }
    }

    std::string EngineFileManager::LoadTextureFile(const std::filesystem::path& path) {
        std::string id = path.lexically_normal().generic_string();
        if (!mTextureCache.Contains(id)) {
            mTextureCache.Add(id, std::make_unique<const Graphics::Resources::Texture>(mRootPath / path));
        }
        return id;
    }

    void EngineFileManager::LoadSound(const FileRequest& request) {
        if (mSoundCache.Contains(request.id)) {
            Logs::Logger::Warn(R"([FILES] Sound "{}" is already cached; "{}" was not loaded.)", request.id, request.path.string());
            return;
        }

        const auto file = ReadFile(mRootPath / request.path);
        auto audio = file.has_value()
            ? SDL::Helpers::LoadAudio(file.value(), Sound::Resources::ClipSampleRate, Sound::Resources::ClipChannels)
            : std::nullopt;
        if (!audio.has_value()) {
            Logs::Logger::Error(R"([FILES] Failed to load sound "{}" from "{}".)", request.id, request.path.string());
            mFailedLoads.push_back(request.id);
            return;
        }

        mSoundCache.Add(request.id, std::make_unique<const Sound::Resources::AudioClip>(std::move(audio->samples)));
    }
}
