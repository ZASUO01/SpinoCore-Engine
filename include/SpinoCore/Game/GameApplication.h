// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "InitializationContext.h"

namespace SpinoCore::Game {
    class GameApplication {
    public:
        virtual ~GameApplication() = default;
        virtual void Initialize(const InitializationContext& ctx) = 0;
    };
}
