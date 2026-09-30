// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include <stop_token>
#include <thread>
#include <vector>
#include "Utils/ThreadSafeQueue.h"

namespace SpinoCore::FileSystem {
    namespace Core {
        class VirtualFileSystem;
    }

    enum class FileOpType {
        READ,
        WRITE
    };

    struct FileRequest {
        uint64_t id;
        FileOpType op;
        std::string virtualPath;
        std::vector<uint8_t> data;
    };

    struct FileResponse {
        uint64_t id;
        FileOpType op;
        bool success;
        std::string virtualPath;
        std::vector<uint8_t> data;
    };

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

        void RequestAsync(const FileRequest &request);
        [[nodiscard]] FileResponse RequestSync(const FileRequest &request) const;

        [[nodiscard]] std::optional<FileResponse> TryPopResponse();
        [[nodiscard]] bool IsBusy() const noexcept;
    private:
        void WorkerRoutine(const std::stop_token& stopToken);

         Utils::ThreadSafeQueue<FileRequest> mRequestQueue;
         Utils::ThreadSafeQueue<FileResponse> mResponseQueue;
         std::unique_ptr<Core::VirtualFileSystem> mFileSystem;
         std::jthread mWorkerThread;
         std::atomic<uint32_t> mPendingTasks{0};
    };
}
