// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "VirtualFileSystem.h"
#include <ranges>
#include "VirtualPath.h"
#include "FileSystem/Mount/MountPoint.h"

namespace SpinoCore::FileSystem::Core {
    VirtualFileSystem::VirtualFileSystem(ConstructorKey) {}
    VirtualFileSystem::~VirtualFileSystem() = default;

    std::unique_ptr<VirtualFileSystem> VirtualFileSystem::Create() {
        return std::make_unique<VirtualFileSystem>(ConstructorKey{});
    }

    bool VirtualFileSystem::Exists(const std::string_view fullVirtualPath) const {
        const auto parsed = ParseVirtualPath(fullVirtualPath);
        if (!parsed || !IsValidVirtualPath(parsed->path)) {
            Logs::Logger::Error("[VIRTUAL FILE SYSTEM] Failed to check if path '{}' exists: malformed path.", fullVirtualPath);
            return false;
        }

        std::lock_guard lock(mMutex);
        const auto mounts = GetMountsForProtocol(parsed->protocol);

        if (!mounts) {
            Logs::Logger::Error(
                "[VIRTUAL FILE SYSTEM] Failed to check if path '{}' exists: the protocol {} is not mounted.",
                fullVirtualPath, parsed->protocol);
            return false;
        }

        for (const auto& mountIt : std::views::reverse(*mounts)) {
            if (mountIt->Exists(parsed->path)) {
                return true;
            }
        }

        return false;
    }

    std::optional<std::vector<uint8_t>> VirtualFileSystem::Read(const std::string_view fullVirtualPath) const {
        const auto parsed = ParseVirtualPath(fullVirtualPath);
        if (!parsed || !IsValidVirtualPath(parsed->path)) {
            Logs::Logger::Error("[VIRTUAL FILE SYSTEM] Failed to read path '{}': malformed path.", fullVirtualPath);
            return std::nullopt;
        }

        std::lock_guard lock(mMutex);
        const auto mounts = GetMountsForProtocol(parsed->protocol);

        if (!mounts) {
            Logs::Logger::Error(
              "[VIRTUAL FILE SYSTEM] Failed to read path '{}': the protocol {} is not mounted.",
              fullVirtualPath, parsed->protocol);
            return std::nullopt;
        }

        for (const auto& mountIt : std::views::reverse(*mounts)) {
            if (auto data = mountIt->Read(parsed->path)) {
                return data;
            }
        }

        Logs::Logger::Error("[VIRTUAL FILE SYSTEM] Failed to read. None of the mounts at '{}' protocol provides the path '{}'.", parsed->protocol, parsed->path);
        return std::nullopt;
    }

    bool VirtualFileSystem::Write(const std::string_view fullVirtualPath, const std::vector<uint8_t>& data) const {
        const auto parsed = ParseVirtualPath(fullVirtualPath);
        if (!parsed || !IsValidVirtualPath(parsed->path)) {
            Logs::Logger::Error("[VIRTUAL FILE SYSTEM] Failed to write on path '{}': malformed path.", fullVirtualPath);
            return false;
        }

        std::lock_guard lock(mMutex);
        const auto mounts = GetMountsForProtocol(parsed->protocol);

        if (!mounts) {
            Logs::Logger::Error(
              "[VIRTUAL FILE SYSTEM] Failed to write on path '{}': the protocol {} is not mounted.",
              fullVirtualPath, parsed->protocol);
            return false;
        }

        for (const auto& mountIt : std::views::reverse(*mounts)) {
            if (!mountIt->IsReadOnly()) {
                const bool success = mountIt->Write(parsed->path, data);
                if (!success) {
                    Logs::Logger::Error("[VIRTUAL FILE SYSTEM] Failed to write on path '{}'.", parsed->path);
                }
                return success;
            }
        }

        Logs::Logger::Error("[VIRTUAL FILE SYSTEM] Failed to write: Every mount at '{}' protocol are read-only.", parsed->protocol);
        return false;
    }

    const std::vector<std::unique_ptr<Mount::MountPoint>>* VirtualFileSystem::GetMountsForProtocol(const std::string_view protocol) const {
        if (mLastMountsCache && mLastProtocolCache == protocol) {
            return mLastMountsCache;
        }

        const auto it = mMounts.find(protocol);
        if (it == mMounts.end()) {
            return nullptr;
        }

        mLastProtocolCache = it->first;
        mLastMountsCache = &it->second;
        return mLastMountsCache;
    }
}