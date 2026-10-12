// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "Core/Engine/CoreModules.h"

namespace SpinoCore::Core::Engine {
    struct CoreModules;
}

namespace SpinoCore::Runtime {
    class FrameOperations final {
    public:
        explicit FrameOperations(Core::Engine::CoreModules& core) noexcept;

        [[nodiscard]] bool ProcessEvents();
        void UpdateSimulation(float deltaTime);
        void InterpolateEntities(float alpha);
        void Render();

    private:
        Core::Engine::CoreModules& mCore;
    };
}
