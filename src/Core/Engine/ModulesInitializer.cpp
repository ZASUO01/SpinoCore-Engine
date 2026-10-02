// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "ModulesInitializer.h"
#include "APIModules.h"
#include "CoreModules.h"
#include "Config/Constants.h"
#include "Config/Helpers/ConfigHelpers.h"
#include "FileSystem/Core/VirtualFileSystem.h"
#include "FileSystem/Mount/FolderMount.h"

namespace SpinoCore::Core::Engine {
    APIModules::~APIModules() = default;
    CoreModules::~CoreModules() = default;

    bool ModulesInitializer::Initialize(APIModules &api, CoreModules &core) {
        if (!InitializeFileModules(api, core)) return false;
        InitializeConfigModule(api, core);

        return true;
    }

    bool ModulesInitializer::InitializeFileModules(APIModules &api, CoreModules &core) {
        using namespace Config::Constants::FileSystem;

        auto vfs = FileSystem::Core::VirtualFileSystem::Create();
        if (!vfs) return false;

        if (!vfs->Mount<FileSystem::Mount::FolderMount>(
            BASE_RESOURCES_PATH.data(),
                    BASE_RESOURCES_PATH.data()
        )) return false;

        if (!vfs->Mount<FileSystem::Mount::FolderMount>(CONFIG_PATH.data(), CONFIG_PATH.data())) return false;
        if (!vfs->Mount<FileSystem::Mount::FolderMount>(ASSETS_PATH.data(), ASSETS_PATH.data())) return false;

        core.fileWorker = FileSystem::FileWorker::Create(std::move(vfs));
        if (!core.fileWorker) return false;

        api.fileManager = FileSystem::FileManager::Create(core);
        if (!api.fileManager) return false;

        return true;
    }

    void ModulesInitializer::InitializeConfigModule(const APIModules &api, CoreModules &core) {
        using namespace Config::Constants::FileSystem;
        using Config::Helpers::RootConfig;

        auto& fileManager = api.fileManager;
        if (!fileManager) return;

        RootConfig config;
        if (const auto rawFile = fileManager->LoadJSONFile(CONFIG_FILE_NAME.data()); rawFile.has_value()) {
            try {
                config = rawFile.value().get<RootConfig>();
            } catch (const nlohmann::json::type_error& e) {
                Logs::Logger::Warn("[MODULES INITIALIZER] JSON config file has incompatible types: {}. Using default values.", e.what());
            } catch (const nlohmann::json::exception& e) {
                Logs::Logger::Warn("[MODULES INITIALIZER] Failed to read JSON config file: {}. Using default values.", e.what());
            }
        }else {
            Logs::Logger::Warn("[MODULES INITIALIZER] Failed to load JSON config file. Using default values.");
        }

        Config::Helpers::SetInitialConfig(config, core.store);

        auto level = core.store.Get("engine.loglevel", 0);
        Logs::Logger::SetLevel(static_cast<Logs::Level>(level));
    }
}
