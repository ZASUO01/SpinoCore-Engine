// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include "Scene.h"

namespace SpinoCore::Core::Scenes {
    class ScenesManager {
    public:
        virtual ~ScenesManager() = default;

        virtual void PushScene(std::unique_ptr<Scene> scene);
        virtual void PopScene();
    };
}
