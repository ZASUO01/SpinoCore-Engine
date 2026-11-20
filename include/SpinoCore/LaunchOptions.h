// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "SpinoCore/Game/GameInfo.h"

namespace SpinoCore {
    struct LaunchOptions final {
        const Game::GameInfo& gameInfo{};
    };
}
