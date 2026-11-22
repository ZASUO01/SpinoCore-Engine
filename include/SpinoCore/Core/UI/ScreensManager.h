// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>

namespace SpinoCore::Core::UI {
    class Screen;

    class ScreensManager {
    public:
        virtual ~ScreensManager() = default;

        virtual void PushScreen(std::unique_ptr<Screen> screen) = 0;
        virtual void PopScreen() = 0;
        virtual void CleanUp() = 0;
    };
}
