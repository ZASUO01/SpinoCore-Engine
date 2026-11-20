// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "EngineOperations.h"
#include "Core/Events/Handler.h"

namespace SpinoCore::Core::Engine {
    EngineOperations::EngineOperations(EngineModules &modules) :mModules(modules){}

    bool EngineOperations::ProcessEvents() {
        return Events::Handler::PollEvents();
    }

    bool EngineOperations::PrepareSimulation() {
        return true;
    }

    void EngineOperations::ProcessInputs() {}

    void EngineOperations::UpdateSimulation(float /*deltaTime*/) {}

    void EngineOperations::Interpolate(float /*alpha*/) {}

    void EngineOperations::Render() {}

    EngineOperations::Duration EngineOperations::GetFramePeriod() {
        return Duration{16'666'665};
    }
}
