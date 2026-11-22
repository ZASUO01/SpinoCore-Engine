// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once

/// @brief The game loop.
namespace SpinoCore::Runtime {
    /// @brief Controls the fixed-step game loop.
    class LoopingManager {
    public:
        virtual ~LoopingManager() = default;
        /// @return The frames per second measured over the last second.
        [[nodiscard]] virtual float GetMeasuredFPS() const noexcept = 0;
        /// @brief Stops the simulation; rendering continues.
        virtual void Pause() noexcept = 0;
        /// @brief Restarts the simulation.
        virtual void Resume() noexcept = 0;
        /// @brief Ends the loop after the current frame.
        virtual void Stop() noexcept = 0;
    };
}