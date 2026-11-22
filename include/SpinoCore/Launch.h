// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include "SpinoCore/Game/GameApplication.h"
#include "LaunchOptions.h"
#include "SpinoCore/Logs/LogOptions.h"

/// @brief Root namespace of the SpinoCore engine.
namespace SpinoCore {
    /// @brief Creates the engine, runs the game until the window closes and shuts everything down.
    ///
    /// Only one engine can run at a time. Errors are logged instead of thrown.
    /// @param game Game to run.
    /// @param launchOptions Window, game metadata and rendering options.
    /// @param logOptions Log level and sink.
    /// @return EXIT_SUCCESS, or EXIT_FAILURE when the engine could not start or stopped with an error.
    [[nodiscard]] int Launch(
        std::unique_ptr<Game::GameApplication> game,
        const LaunchOptions& launchOptions = {},
        Logs::LogOptions logOptions = {}
    );
}
