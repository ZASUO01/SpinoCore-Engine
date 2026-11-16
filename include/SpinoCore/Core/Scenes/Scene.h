// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once

namespace SpinoCore::Core::Scenes {
    class Scene {
    public:
        virtual ~Scene() = default;

        virtual void OnStart() = 0;

    };
}
