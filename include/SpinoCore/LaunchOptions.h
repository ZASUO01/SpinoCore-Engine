// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "Core/Window/WindowProps.h"
#include "SpinoCore/Game/GameInfo.h"

namespace SpinoCore {
    struct LaunchOptions final {
        Game::GameInfo gameInfo{};
        Core::Window::WindowProps windowProps{};
    };
}
