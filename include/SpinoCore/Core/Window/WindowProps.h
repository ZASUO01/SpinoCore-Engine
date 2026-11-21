// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <string>
#include "WindowSize.h"

namespace SpinoCore::Core::Window {
    enum class WindowMode {
        Windowed,
        BorderlessFullscreen,
        Fullscreen,
    };

    struct WindowProps final {
        std::string title{"SpinoCore Engine"};
        Size2D size{ WINDOW_SIZE_SMALL };
        WindowMode windowMode{ WindowMode::Windowed };
        bool hideSystemCursor{false};
    };
}
