// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <string_view>

namespace SpinoCore::FileSystem {
    class FileLoader {
    public:
        virtual ~FileLoader() = default;
        virtual void LoadFiles(std::string_view manifestPath) = 0;
    };
}
