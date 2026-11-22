// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once

namespace SpinoCore::Core::Scenes {
    struct StartContext;
    struct ControlContext;

    /// @brief Base class of game scenes.
    ///
    /// OnStart runs when the scene is pushed and should only request assets.
    /// OnSetup runs once those assets are loaded, OnUpdate runs on every simulation step,
    /// and OnFinish runs when the scene is replaced, popped or destroyed.
    class Scene {
    public:
        virtual ~Scene() = default;

        /// @brief Requests the assets of the scene.
        /// @param ctx Access to the file manager.
        virtual void OnStart(const StartContext& ctx) = 0;
        /// @brief Creates screens, cameras, actors and sounds after the assets are loaded.
        /// @param ctx Access to the engine modules.
        virtual void OnSetup(const ControlContext& ctx) = 0;
        /// @brief Runs the scene logic on each simulation step.
        /// @param ctx Access to the engine modules and the simulation input.
        virtual void OnUpdate(const ControlContext& ctx) = 0;
        /// @brief Runs before the engine cleans up the modules used by the scene.
        /// @param ctx Access to the engine modules.
        virtual void OnFinish(const ControlContext& ctx) = 0;
    };
}
