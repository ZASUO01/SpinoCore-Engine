// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <filesystem>
#include <shared_mutex>
#include "MountPoint.h"

namespace SpinoCore::FileSystem::Mount {
    class FolderMount final : public MountPoint {
    public:
        class ConstructorKey { friend class FolderMount; ConstructorKey() = default; };
        FolderMount(ConstructorKey, std::filesystem::path rootPath, bool isReadOnly);
        ~FolderMount() override;

        FolderMount(const FolderMount&) = delete;
        FolderMount& operator=(const FolderMount&) = delete;
        FolderMount(FolderMount&&) = delete;
        FolderMount& operator=(FolderMount&&) = delete;

        [[nodiscard]] static std::unique_ptr<FolderMount> Create(std::filesystem::path rootPath, bool isReadOnly = true);

        [[nodiscard]] bool Exists(std::string_view localPath) const override;
        [[nodiscard]] std::optional<std::vector<uint8_t>> Read(std::string_view localPath) const override;
        [[nodiscard]] bool Write(std::string_view localPath, const std::vector<uint8_t> &data) override;
        [[nodiscard]] bool IsReadOnly() const override { return mIsReadOnly; }

    protected:
        [[nodiscard]] bool Initialize() override;

    private:
        [[nodiscard]] std::optional<std::filesystem::path> ResolvePhysicalPath(std::string_view localPath) const;

        mutable std::shared_mutex mMountMutex;
        std::filesystem::path mRootPath;
        bool mIsReadOnly;
        bool mIsMounted{false};
    };
}