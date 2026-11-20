// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "SpinoCore/Game/GameInfo.h"
#include "SpinoCore/Core/Window/WindowProps.h"

namespace SpinoCore {
    struct LaunchOptions final {
        Game::GameInfo gameInfo{};
        Core::Window::WindowProps windowProps{};
    };
}
