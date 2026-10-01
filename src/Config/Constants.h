// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "Utils/ConstString.h"

namespace SpinoCore::Config::Constants::FileSystem {
    using Utils::ConstString;

    inline constexpr auto PATH_SEPARATOR = ConstString("://");
    inline constexpr auto BASE_RESOURCES_PATH = ConstString("@base-resources");
    inline constexpr auto BASE_MANIFEST_PATH = BASE_RESOURCES_PATH + PATH_SEPARATOR + ConstString("manifest.json");
    inline constexpr auto MANIFEST_FILE_TYPE = ConstString("MANIFEST_FILE");
    inline constexpr auto MANIFEST_FILE_VERSION = ConstString("1.0.0");
    inline constexpr auto CONFIG_PATH = ConstString("@config");
    inline constexpr auto CONFIG_FILE_NAME = CONFIG_PATH +  PATH_SEPARATOR + ConstString("config.json");
    inline constexpr auto ASSETS_PATH = ConstString("@assets");
}

