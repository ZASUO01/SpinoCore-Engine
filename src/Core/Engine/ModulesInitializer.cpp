// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "ModulesInitializer.h"
#include "APIModules.h"
#include "CoreModules.h"
#include "FileSystem/Core/VirtualFileSystem.h"
#include "FileSystem/Mount/FolderMount.h"

namespace SpinoCore::Core::Engine {
    APIModules::~APIModules() = default;
    CoreModules::~CoreModules() = default;

    bool ModulesInitializer::Initialize(APIModules &api, CoreModules &core) {
        return true;
    }

    bool ModulesInitializer::InitializeFileModules(APIModules &api, CoreModules &core) {
        auto vfs = FileSystem::Core::VirtualFileSystem::Create();
        if (!vfs) return false;

        //if (!vfs->Mount<FileSystem::Mount::FolderMount>()) return false;

        return true;
    }
}
