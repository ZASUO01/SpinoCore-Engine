// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "SpinoCore/FileSystem/FileLoader.h"

namespace SpinoCore::FileSystem {
    class FileManager final : public FileLoader {
    public:
        
        void LoadFiles(std::string_view manifestPath) override;
    };
}
