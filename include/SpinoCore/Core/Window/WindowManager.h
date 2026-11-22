// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "WindowProps.h"
#include "WindowSize.h"

namespace SpinoCore::Core::Window {
    /// @brief Controls the game window.
    class WindowManager {
    public:
        virtual ~WindowManager() = default;
        /// @return The current window mode.
        [[nodiscard]] virtual WindowMode GetMode() const noexcept = 0;
        /// @brief Changes the window mode.
        /// @param mode New mode.
        /// @return Whether the mode was applied.
        virtual bool SetMode(WindowMode mode) noexcept = 0;
        /// @return The window size in pixels.
        [[nodiscard]] virtual Size2D GetSize() const noexcept = 0;
        /// @brief Resizes the window.
        /// @param size One of the WINDOW_SIZE_* sizes that fits the display.
        /// @return Whether the size was applied.
        virtual bool SetSize(Size2D size) noexcept = 0;
    };
}