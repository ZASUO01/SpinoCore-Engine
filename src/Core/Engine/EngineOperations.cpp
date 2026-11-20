// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "EngineOperations.h"
#include "EngineModules.h"
#include "Core/Events/Handler.h"

namespace SpinoCore::Core::Engine {
    EngineOperations::EngineOperations(EngineModules &modules) :mModules(modules){}

    bool EngineOperations::ProcessEvents() {
        return Events::Handler::PollEvents(mModules.eventBus);
    }

    bool EngineOperations::PrepareSimulation() {
        return true;
    }

    void EngineOperations::ProcessFrameInputs() {}

    void EngineOperations::ProcessSimulationInputs() {}

    void EngineOperations::UpdateSimulation(float /*deltaTime*/) {}

    void EngineOperations::Interpolate(float /*alpha*/) {}

    void EngineOperations::Render() {
        mModules.renderer->Draw();
    }

    EngineOperations::Duration EngineOperations::GetFramePeriod() {
        return mModules.renderer->GetFramePeriod();
    }
}
