// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once

namespace SpinoCore::Core::Scenes {
    struct StartContext;
    struct ControlContext;

    class Scene {
    public:
        virtual ~Scene() = default;

        virtual void OnStart(const StartContext& ctx) = 0;
        virtual void OnSetup(const ControlContext& ctx) = 0;
        virtual void OnUpdate(const ControlContext& ctx) = 0;
        virtual void OnFinish(const ControlContext& ctx) = 0;
    };
}
