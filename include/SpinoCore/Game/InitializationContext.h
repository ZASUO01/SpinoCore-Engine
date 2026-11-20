// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "SpinoCore/Core/Window/WindowManager.h"
#include "SpinoCore/Core/Scenes/ScenesManager.h"

namespace SpinoCore::Game {
    struct InitializationContext final {
        Core::Window::WindowManager& windowManager;
        Core::Scenes::ScenesManager& scenesManager;
    };
}
