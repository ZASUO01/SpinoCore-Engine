// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "Defs/StaticCreateConcept.h"
#include "SpinoCore/Logs/Logger.h"

namespace SpinoCore::FileSystem::Mount {
    class MountPoint;
}

namespace SpinoCore::FileSystem::Core {
    struct StringHash {
        using is_transparent = void;
        size_t operator()(const std::string_view txt) const {
            return std::hash<std::string_view>{}(txt);
        }
    };

    class VirtualFileSystem {
    public:
        class ConstructorKey{ friend class VirtualFileSystem; ConstructorKey() = default; };
        explicit VirtualFileSystem(ConstructorKey);
        ~VirtualFileSystem();

        VirtualFileSystem(const VirtualFileSystem&) = delete;
        VirtualFileSystem& operator=(const VirtualFileSystem&) = delete;
        VirtualFileSystem(VirtualFileSystem&&) = delete;
        VirtualFileSystem& operator=(VirtualFileSystem&&) = delete;

        [[nodiscard]] static std::unique_ptr<VirtualFileSystem> Create();

        template <typename TMount, typename... Args>
        requires Defs::HasStaticCreate<TMount, Args...>
        [[nodiscard]] bool Mount(const std::string_view protocol, Args&&... args) {
            if (protocol.empty()) {
                Logs::Logger::Error("[VIRTUAL FILE SYSTEM] Failed to mount with empty protocol.");
                return false;
            }

            std::unique_ptr<TMount> mount = TMount::Create(std::forward<Args>(args)...);
            if (!mount) return false;

            std::unique_lock lock(mMutex);

            auto it = mMounts.find(protocol);
            if (it == mMounts.end()) {
                it = mMounts.emplace(std::string(protocol), std::vector<std::unique_ptr<Mount::MountPoint>>{}).first;
            }
            it->second.push_back(std::move(mount));

            return true;
        }

        [[nodiscard]] bool Exists(std::string_view fullVirtualPath) const;
        [[nodiscard]] std::optional<std::vector<uint8_t>> Read(std::string_view fullVirtualPath) const;
        [[nodiscard]] bool Write(std::string_view fullVirtualPath, const std::vector<uint8_t>& data) const;

    private:
        [[nodiscard]] const std::vector<std::unique_ptr<Mount::MountPoint>>* GetMountsForProtocol(std::string_view protocol) const;

        mutable std::shared_mutex mMutex;

        std::unordered_map<
            std::string,
            std::vector<std::unique_ptr<Mount::MountPoint>>,
            StringHash,
            std::equal_to<>
        > mMounts;
    };
}