// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

namespace SpinoCore::FileSystem::Mount {
    class MountPoint {
    public:
        virtual ~MountPoint() = default;
        [[nodiscard]] virtual bool Exists(std::string_view localPath) const = 0;
        [[nodiscard]] virtual std::optional<std::vector<uint8_t>> Read(std::string_view localPath) const = 0;
        [[nodiscard]] virtual bool Write(std::string_view localPath, const std::vector<uint8_t>& data) = 0;
        [[nodiscard]] virtual bool IsReadOnly() const = 0;

    protected:
        [[nodiscard]] virtual bool Initialize() = 0;
    };
}