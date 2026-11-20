// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <chrono>
#include "Runtime/FrameOperations.h"

namespace SpinoCore::Core::Engine {
    struct EngineModules;

    class EngineOperations final : public Runtime::FrameOperations {
    public:
        using Duration = std::chrono::nanoseconds;

        explicit EngineOperations(EngineModules& modules);
        ~EngineOperations() override = default;

        EngineOperations(const EngineOperations&) = delete;
        EngineOperations& operator=(const EngineOperations&) = delete;
        EngineOperations(EngineOperations&&) = delete;
        EngineOperations& operator=(EngineOperations&&) = delete;

        [[nodiscard]] bool ProcessEvents() override;
        [[nodiscard]] bool PrepareSimulation() override;
        void ProcessFrameInputs() override;
        void ProcessSimulationInputs() override;
        void UpdateSimulation(float deltaTime) override;
        void Interpolate(float alpha) override;
        void Render() override;
        Duration GetFramePeriod() override;

    private:
        EngineModules& mModules;
    };
}
