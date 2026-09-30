// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "FileWorker.h"
#include "FileSystem/Core/VirtualFileSystem.h"

namespace SpinoCore::FileSystem {
    FileWorker::FileWorker(ConstructorKey, std::unique_ptr<Core::VirtualFileSystem> vfs) :mVfs(std::move(vfs)) {}
    FileWorker::~FileWorker() = default;

    std::unique_ptr<FileWorker> FileWorker::Create(std::unique_ptr<Core::VirtualFileSystem> vfs) {
        return std::make_unique<FileWorker>(ConstructorKey{}, std::move(vfs));
    }
}
