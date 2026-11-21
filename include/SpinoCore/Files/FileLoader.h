// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <string>

namespace SpinoCore::Files {
    class FileLoader {
    public:
        virtual ~FileLoader() = default;

        void LoadFromManifest(const std::string& manifestPath);
        void LoadFromSinglePath(const std::string& path);
    };
}
