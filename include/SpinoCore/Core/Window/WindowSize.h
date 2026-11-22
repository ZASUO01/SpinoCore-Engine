// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "SpinoCore/Utils/Size2D.h"

namespace SpinoCore::Core::Window {
    using Utils::Size2D;

    /// Largest window size, 1920x1080.
    inline constexpr Size2D WINDOW_SIZE_MAX = { .width = 1920, .height = 1080 };
    /// 1600x900 window size.
    inline constexpr Size2D WINDOW_SIZE_EXTRA_LARGE = { .width = 1600, .height = 900  };
    /// 1366x768 window size.
    inline constexpr Size2D WINDOW_SIZE_LARGE = { .width = 1366, .height = 768  };
    /// 1280x720 window size.
    inline constexpr Size2D WINDOW_SIZE_MEDIUM = { .width = 1280, .height = 720  };
    /// 1024x768 window size, the default.
    inline constexpr Size2D WINDOW_SIZE_SMALL = { .width = 1024, .height = 768  };
    /// Smallest window size, 800x600.
    inline constexpr Size2D WINDOW_SIZE_MIN = { .width = 800,  .height = 600  };
}