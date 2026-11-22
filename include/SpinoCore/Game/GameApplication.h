// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "InitializationContext.h"

/// @brief The entry point a game implements and the information about it.
namespace SpinoCore::Game {
    /// @brief Base class of a game; the engine calls Initialize once after creating the window.
    class GameApplication {
    public:
        virtual ~GameApplication() = default;
        /// @brief Pushes the first scene and adjusts the window.
        /// @param ctx Access to the window and the scenes manager.
        virtual void Initialize(const InitializationContext& ctx) = 0;
    };
}
