// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include "Screen.h"

namespace SpinoCore::Core::UI {
    class ScreensManager {
    public:
        virtual ~ScreensManager() = default;

        virtual void PushScreen(std::unique_ptr<Screen> screen) = 0;
        virtual void PopScreen() = 0;
    };
}
