// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "FolderMount.h"
#include "FileSystem/Helpers/FileHelpers.h"
#include "SpinoCore/Logs/Logger.h"

namespace SpinoCore::FileSystem::Mount {
    FolderMount::FolderMount(ConstructorKey, std::filesystem::path rootPath, const bool isReadOnly)
    :mRootPath(std::move(rootPath)), mIsReadOnly(isReadOnly)
    {}
    FolderMount::~FolderMount() = default;

    std::unique_ptr<FolderMount> FolderMount::Create(std::filesystem::path rootPath, bool isReadOnly) {
        auto mount = std::make_unique<FolderMount>(ConstructorKey{}, std::move(rootPath), isReadOnly);

        if (!mount->Initialize()) return nullptr;

        return mount;
    }

    bool FolderMount::Exists(const std::string_view localPath) const {
        if (!mIsMounted) {
            Logs::Logger::Warn("[FOLDER MOUNT] Cannot verify if the path '{}' exists on not initialized mount.", localPath);
            return false;
        }

        const auto targetPath = ResolvePhysicalPath(localPath);
        if (!targetPath) return false;

        std::error_code ec;
        const bool exists = std::filesystem::exists(*targetPath, ec);
        const bool isRegular = std::filesystem::is_regular_file(*targetPath, ec);

        return exists && !ec && isRegular;
    }

    std::optional<std::vector<uint8_t> > FolderMount::Read(const std::string_view localPath) const {
        if (!mIsMounted) {
            Logs::Logger::Error("[FOLDER MOUNT] Attempted to read on path '{}' before init.", localPath);
            return std::nullopt;
        }

        const auto targetPath = ResolvePhysicalPath(localPath);
        if (!targetPath || !Exists(localPath)) {
            Logs::Logger::Error("[FOLDER MOUNT] Failed to read. The path '{}' is either inexistent or invalid.", localPath);
            return std::nullopt;
        }

        return Helpers::ReadFile(*targetPath);
    }

    bool FolderMount::Write(const std::string_view localPath, const std::vector<uint8_t> &data) {
        if (!mIsMounted || mIsReadOnly) {
            Logs::Logger::Error("[FOLDER MOUNT] Denied write on path '{}'. The mount is either not initialized or read-only.", localPath);
            return false;
        }

        const auto targetPath = ResolvePhysicalPath(localPath);
        if (!targetPath) return false;

        std::error_code ec;
        const auto parentPath = targetPath->parent_path();

        std::filesystem::create_directories(parentPath, ec);
        if (ec) {
            Logs::Logger::Error("[FOLDER MOUNT] Failed to create directories: {} - {}", parentPath.string(), ec.message());
            return false;
        }

        return Helpers::WriteToFile(*targetPath, data);
    }

    bool FolderMount::Initialize() {
        std::error_code ec;
        if (!std::filesystem::exists(mRootPath, ec) || ec || !std::filesystem::is_directory(mRootPath, ec)) {
            Logs::Logger::Error("[FOLDER MOUNT] Root path is either not valid or not a directory: {}", mRootPath.string());
            return false;
        }

        mRootPath = std::filesystem::weakly_canonical(mRootPath, ec);
        if (ec) {
            Logs::Logger::Error("[FOLDER MOUNT] Failed to resolve canonical path: {}", ec.message());
            return false;
        }

        mIsMounted = true;
        return true;
    }

    std::optional<std::filesystem::path> FolderMount::ResolvePhysicalPath(const std::string_view localPath) const {
        if (localPath.empty() || localPath.find("..") != std::string_view::npos) {
            Logs::Logger::Error("[FOLDER MOUNT] Could not resolve the path '{}': It's either empty or tries a traversal.", localPath);
            return std::nullopt;
        }

        const std::filesystem::path relativeTarget(localPath);
        if (relativeTarget.is_absolute()) {
            Logs::Logger::Error("[FOLDER MOUNT] Could not resolve the path '{}': Absolute paths are not allowed.", localPath);
            return std::nullopt;
        }

        std::error_code ec;
        auto resolved = std::filesystem::weakly_canonical(mRootPath / relativeTarget, ec);

        if (ec) {
            Logs::Logger::Error("[FOLDER MOUNT] Internal error while evaluating canonical path '{}': {}", localPath, ec.message());
            return std::nullopt;
        }
        return resolved;
    }
}