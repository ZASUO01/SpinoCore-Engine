// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "SpinoCore/Core/Lighting/GlobalLight.h"
#include "SpinoCore/Game/GameInfo.h"
#include "SpinoCore/Core/Window/WindowProps.h"

namespace SpinoCore {
    /// @brief Options used when the engine starts.
    struct LaunchOptions final {
        /// Application metadata.
        Game::GameInfo gameInfo{};
        /// Window settings.
        Core::Window::WindowProps windowProps{};
        /// Turns on lighting and shadows.
        bool enableGlobalLight{false};
        /// Initial light settings.
        Core::Lighting::GlobalLightSettings globalLight{};
        /// Draws the outlines of the colliders.
        bool debugColliders{false};
    };
}
