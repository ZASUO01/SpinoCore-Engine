// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "Utils/ConstString.h"

namespace SpinoCore::Config::Constants::FileSystem {
    inline constexpr auto BASE_RESOURCES_PATH = Utils::ConstString("@base-resources");
    inline constexpr auto BASE_MANIFEST_PATH = BASE_RESOURCES_PATH + Utils::ConstString("://manifest.json");
    inline constexpr auto MANIFEST_FILE_TYPE = Utils::ConstString("MANIFEST_FILE");
    inline constexpr auto MANIFEST_FILE_VERSION = Utils::ConstString("1.0.0");
}

