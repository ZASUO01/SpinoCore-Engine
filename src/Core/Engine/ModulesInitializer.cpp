// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "ModulesInitializer.h"
#include "APIModules.h"
#include "CoreModules.h"
#include "Config/Constants.h"
#include "FileSystem/Core/VirtualFileSystem.h"
#include "FileSystem/Mount/FolderMount.h"

namespace SpinoCore::Core::Engine {
    APIModules::~APIModules() = default;
    CoreModules::~CoreModules() = default;

    bool ModulesInitializer::Initialize(APIModules &api, CoreModules &core) {
        if (!InitializeFileModules(api, core)) return false;

        InitializeConfigModule(core);

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

    void ModulesInitializer::InitializeConfigModule(const CoreModules &core) {
        auto& fileWorker = core.fileWorker;
        if (!fileWorker) return;


    }
}
