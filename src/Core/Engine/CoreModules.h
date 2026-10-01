// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include "FileSystem/FileWorker.h"
#include "SpinoCore/Config/Store.h"

namespace SpinoCore::Core::Engine {
    struct CoreModules final {
        std::unique_ptr<FileSystem::FileWorker> fileWorker{nullptr};
        Config::Store store{};

        ~CoreModules();
    };
}
