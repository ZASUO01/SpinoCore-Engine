// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "FileSystem/FileManager.h"

namespace SpinoCore::Core::Engine {
    struct APIModules final {
        std::unique_ptr<FileSystem::FileManager> fileManager{nullptr};

        ~APIModules();
    };
}
