// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "SpinoCore/Core/Window/WindowManager.h"
#include "SpinoCore/Core/Scenes/ScenesManager.h"

namespace SpinoCore::Game {
    /// @brief Modules available to GameApplication::Initialize.
    struct InitializationContext final {
        /// Window control.
        Core::Window::WindowManager& windowManager;
        /// Scene stack, used to push the first scene.
        Core::Scenes::ScenesManager& scenesManager;
    };
}
