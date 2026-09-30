// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "FileWorker.h"
#include "FileSystem/Core/VirtualFileSystem.h"

namespace SpinoCore::FileSystem {
    FileWorker::FileWorker(ConstructorKey, std::unique_ptr<Core::VirtualFileSystem> vfs) :mFileSystem(std::move(vfs)) {
        mWorkerThread = std::jthread([this](const std::stop_token &st) {
              WorkerRoutine(st);
        });
    }
    FileWorker::~FileWorker() = default;

    std::unique_ptr<FileWorker> FileWorker::Create(std::unique_ptr<Core::VirtualFileSystem> vfs) {
        return std::make_unique<FileWorker>(ConstructorKey{}, std::move(vfs));
    }

    void FileWorker::RequestAsync(const FileRequest &request) {
        ++mPendingTasks;
        mRequestQueue.Push(request);
    }

    FileResponse FileWorker::RequestSync(const FileRequest &request) const {
        FileResponse res = {
            .id = request.id,
            .op = request.op,
            .success = false,
            .virtualPath = request.virtualPath
        };

        if (request.op == FileOpType::READ) {
            if (auto fsResponse = mFileSystem->Read(request.virtualPath)) {
                res.success = true;
                res.data = std::move(*fsResponse);
            }
        } else if (request.op == FileOpType::WRITE) {
            if (mFileSystem->Write(request.virtualPath, request.data)) {
                res.success = true;
            }
        }

        return res;
    }

    void FileWorker::WorkerRoutine(const std::stop_token& stopToken) {
        while (!stopToken.stop_requested()) {
            auto reqOpt = mRequestQueue.WaitAndPop(stopToken);
            if (!reqOpt) {
                return;
            }

            auto&[id, op, virtualPath, data] = *reqOpt;
            FileResponse res = {
                .id = id,
                .op = op,
                .success = false,
                .virtualPath = std::move(virtualPath)
            };

            if (op == FileOpType::READ) {
                if (auto fsResponse = mFileSystem->Read(res.virtualPath)) {
                    res.success = true;
                    res.data = std::move(*fsResponse);
                }
            } else if (op == FileOpType::WRITE) {
                if (mFileSystem->Write(res.virtualPath, data)) {
                    res.success = true;
                }
            }

            mResponseQueue.Push(std::move(res));
            --mPendingTasks;
        }
    }
}
