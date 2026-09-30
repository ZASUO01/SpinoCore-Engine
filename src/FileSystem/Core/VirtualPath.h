// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <optional>
#include <string_view>

namespace SpinoCore::FileSystem::Core {
    struct VirtualPath {
        std::string_view protocol;
        std::string_view path;
    };

    [[nodiscard]] std::optional<VirtualPath> ParseVirtualPath(std::string_view fullVirtualPath);
    [[nodiscard]] bool IsValidVirtualPath(std::string_view virtualPath);
}