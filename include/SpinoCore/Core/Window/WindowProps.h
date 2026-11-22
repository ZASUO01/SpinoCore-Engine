// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <string>
#include "WindowSize.h"

/// @brief Window settings and control.
namespace SpinoCore::Core::Window {
    /// @brief How the window is shown.
    enum class WindowMode {
        /// A normal window with borders.
        Windowed,
        /// A borderless window that covers the display.
        BorderlessFullscreen,
        /// Exclusive fullscreen.
        Fullscreen,
    };

    /// @brief Settings used to create the window.
    struct WindowProps final {
        /// Title bar text.
        std::string title{"SpinoCore Engine"};
        /// Initial size; it is reduced to the largest standard size that fits the display.
        Size2D size{ WINDOW_SIZE_SMALL };
        /// Initial mode.
        WindowMode windowMode{ WindowMode::Windowed };
        /// Hides the system cursor, for games that draw their own.
        bool hideSystemCursor{false};
    };
}
