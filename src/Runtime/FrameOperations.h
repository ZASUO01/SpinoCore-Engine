// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <chrono>

namespace SpinoCore::Runtime {
    class FrameOperations {
    public:
        using Duration = std::chrono::nanoseconds;

        virtual ~FrameOperations() = default;

        [[nodiscard]] virtual bool ProcessEvents() = 0;
        [[nodiscard]] virtual bool PrepareSimulation() = 0;
        virtual void ProcessFrameInputs() = 0;
        virtual void ProcessSimulationInputs() = 0;
        virtual void UpdateSimulation(float deltaTime) = 0;
        virtual void Interpolate(float alpha) = 0;
        virtual void Render() = 0;
        [[nodiscard]] virtual Duration GetFramePeriod() = 0;
    };
}
