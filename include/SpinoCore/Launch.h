// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include "SpinoCore/Game/GameApplication.h"
#include "LaunchOptions.h"

namespace SpinoCore {
    [[nodiscard]] int Launch(
        std::unique_ptr<Game::GameApplication> game,
        const LaunchOptions& launchOptions = {}
    );
}
