// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "EngineModules.h"
#include "Core/Events/EventBus.h"
#include "Runtime/FrameOperations.h"

namespace SpinoCore::Core::Engine {
    class EngineOperations final : public Runtime::FrameOperations {
    public:
        explicit EngineOperations(EngineModules& modules, Events::EventBus& bus);
        ~EngineOperations() override;

        [[nodiscard]] bool ProcessEvents() override;
        [[nodiscard]] bool PrepareSimulation() override;
        void ProcessInputs() override;
        void UpdateSimulation(float deltaTime) override;
        void Interpolate(float alpha) override;
        void Render() override;
        [[nodiscard]] Duration GetFramePeriod() override;

    private:
        EngineModules& mModules;
        Events::EventBus& mEventBus;
    };
}
