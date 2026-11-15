// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "Runtime/FrameOperations.h"

namespace SpinoCore::Core::Engine {
    class EngineOperations final : public Runtime::FrameOperations {
    public:
        EngineOperations();
        ~EngineOperations() override;

        [[nodiscard]] bool ProcessEvents() override;
        [[nodiscard]] bool PrepareSimulation() override;
        void ProcessInputs() override;
        void UpdateSimulation(float deltaTime) override;
        void Interpolate(float alpha) override;
        void Render() override;
        [[nodiscard]] Duration GetFramePeriod() override;
    };
}
