// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <array>
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

namespace SpinoCore::Config::Constants::Logs {
    inline constexpr auto LOG_LEVEL = 1;
}

namespace SpinoCore::Config::Constants::App {
    using Utils::ConstString;

    inline constexpr auto NAME = ConstString("SpinoCore Engine");
    inline constexpr auto VERSION = ConstString("0.1.0");
    inline constexpr auto IDENTIFIER = ConstString("SpinoCore Engine");
    inline constexpr auto CREATOR = ConstString("ZASUO01");
    inline constexpr auto URL = ConstString("https://github.com/ZASUO01/SpinoCore-Engine");
}

namespace SpinoCore::Config::Constants::Window {
    using Utils::ConstString;

    struct Resolution {
        int width;
        int height;
    };

    inline constexpr int INITIAL_WIDTH = 1024;
    inline constexpr int INITIAL_HEIGHT = 768;

    inline constexpr std::array<Resolution, 6> STANDARD_RESOLUTIONS {{
        { .width = 1920, .height = 1080 },
        { .width = 1600, .height = 900  },
        { .width = 1366, .height = 768  },
        { .width = 1280, .height = 720  },
        { .width = 1024, .height = 768  },
        {.width = 800,.height = 600},
    }};

    inline constexpr Resolution MINIMUM_RESOLUTION = { .width = INITIAL_WIDTH, .height = INITIAL_HEIGHT };
    inline constexpr auto WINDOW_TITLE = ConstString("SpinoCore Engine");
}