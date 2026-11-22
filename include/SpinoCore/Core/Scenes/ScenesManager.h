// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>

#include "Scene.h"

namespace SpinoCore::Core::Scenes {
    /// @brief Stack of scenes; only the top scene runs.
    class ScenesManager {
    public:
        virtual ~ScenesManager() = default;

        /// @brief Finishes the current scene, cleans up its modules and starts a new one.
        /// @param scene Scene to own; nullptr is ignored.
        virtual void PushScene(std::unique_ptr<Scene> scene) = 0;
        /// @brief Finishes and destroys the top scene; the scene below is set up again.
        virtual void PopScene() = 0;
    };
}
