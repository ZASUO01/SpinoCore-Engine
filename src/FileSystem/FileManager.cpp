// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "FileManager.h"

#include "Config/Constants.h"
#include "Core/Engine/CoreModules.h"
#include "SpinoCore/Logs/Logger.h"
#include "nlohmann/json.hpp"
#include "Utils/JSON/JSONUtils.h"

namespace SpinoCore::FileSystem {
    FileManager::FileManager(ConstructorKey, SpinoCore::Core::Engine::CoreModules &coreModules) :mCoreModules(coreModules){}
    FileManager::~FileManager() = default;

    std::unique_ptr<FileManager> FileManager::Create(SpinoCore::Core::Engine::CoreModules &coreModules) {
        return std::make_unique<FileManager>(ConstructorKey{}, coreModules);
    }

    void FileManager::LoadFiles(std::string_view manifestPath) {

    }

    bool FileManager::LoadFilesSync(const std::string_view manifestPath) const {
        if (manifestPath.empty()) {
            Logs::Logger::Error("[FILE MANAGER] Empty manifest path.");
            return false;
        }

        const std::vector<std::string> requiredFields = {"type", "version", "files" };
        const auto JSONFile = LoadJSONFile(manifestPath, requiredFields);

        if (!JSONFile) return false;

        auto JSONObject = JSONFile.value();

        if (!IsValidFileManifest(JSONObject)) return false;

        auto files = JSONObject["files"];
        for (auto it = files.begin(); it != files.end(); ++it) {

        }

        return true;
    }

    std::optional<json> FileManager::LoadJSONFile(std::string_view manifestPath, const std::vector<std::string>& requiredFields) const {
        const auto& fileWorker = mCoreModules.fileWorker;
        if (!fileWorker) {
            Logs::Logger::Error("[FILE MANAGER] File Worker not initialized.");
            return std::nullopt;
        }

        const FileRequest request = { .id = 0, .op = FileOpType::READ, .virtualPath = manifestPath.data() };
        const auto response = fileWorker->RequestSync(request);

        if (!response.success) return std::nullopt;

        return Utils::JSON::ParseJSONFromBytes(response.data, requiredFields);
    }

    [[nodiscard]] bool FileManager::IsValidFileManifest(json& inJSON) {
        using namespace Config::Constants::FileSystem;

        if (!inJSON.is_object()) {
            Logs::Logger::Error("[FILE MANAGER] JSON file must be an object.");
            return false;
        }

        if (const auto type = inJSON["type"].get<std::string>(); type != MANIFEST_FILE_TYPE.data()) {
            Logs::Logger::Error("[FILE MANAGER] This JSON file type is not supported.");
            return false;
        }

        if (const auto version = inJSON["version"].get<std::string>(); version != MANIFEST_FILE_VERSION.data()) {
            Logs::Logger::Error("[FILE MANAGER] JSON file Version mismatch.");
            return false;
        }

        if (const auto files = inJSON["files"]; !files.is_array()) {
            Logs::Logger::Error("[FILE MANAGER] JSON files must be an array.");
            return false;
        }

        return true;
    }
}
