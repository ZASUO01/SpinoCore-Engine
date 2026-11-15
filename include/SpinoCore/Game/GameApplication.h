// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once

namespace SpinoCore::Game {
    struct InitializationContext;

    class GameApplication {
    public:
        virtual ~GameApplication() = default;
        virtual void Initialize(const InitializationContext& ctx) = 0;
    };
}