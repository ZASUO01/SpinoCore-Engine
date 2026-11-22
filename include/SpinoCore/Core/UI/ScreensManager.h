// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>

namespace SpinoCore::Core::UI {
    class Screen;

    /// @brief Stack of UI screens.
    class ScreensManager {
    public:
        virtual ~ScreensManager() = default;

        /// @brief Pauses the top screen and pushes a new one.
        /// @param screen Screen to own; nullptr is ignored.
        virtual void PushScreen(std::unique_ptr<Screen> screen) = 0;
        /// @brief Exits and destroys the top screen and resumes the one below.
        virtual void PopScreen() = 0;
        /// @brief Exits and destroys every screen.
        virtual void CleanUp() = 0;
    };
}
