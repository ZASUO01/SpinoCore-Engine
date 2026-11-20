// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "StartContext.h"

namespace SpinoCore::Core::Scenes {
    class Scene {
    public:
        virtual ~Scene() = default;

        void Start(const StartContext& ctx);

    protected:
        virtual void OnStart(const StartContext& ctx) = 0;
    };
}
