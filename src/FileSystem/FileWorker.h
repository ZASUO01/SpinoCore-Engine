// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>

namespace SpinoCore::FileSystem {
    namespace Core {
        class VirtualFileSystem;
    }

    class FileWorker final {
    public:
        class ConstructorKey{ friend class FileWorker; ConstructorKey() = default; };
        explicit FileWorker(ConstructorKey, std::unique_ptr<Core::VirtualFileSystem> vfs);
        ~FileWorker();

        FileWorker(const FileWorker&) = delete;
        FileWorker& operator=(const FileWorker&) = delete;
        FileWorker(FileWorker&&) = delete;
        FileWorker& operator=(FileWorker&&) = delete;

        [[nodiscard]] static std::unique_ptr<FileWorker> Create(std::unique_ptr<Core::VirtualFileSystem> vfs);

    private:
        std::unique_ptr<Core::VirtualFileSystem> mVfs;
    };
}
