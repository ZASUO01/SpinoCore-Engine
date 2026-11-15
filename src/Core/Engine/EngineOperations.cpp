// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "EngineOperations.h"
#include "Core/Events/Handler.h"

namespace SpinoCore::Core::Engine {
    EngineOperations::EngineOperations(EngineModules& modules, Events::EventBus& bus)
    :mModules(modules)
    ,mEventBus(bus)
    {}
    EngineOperations::~EngineOperations() = default;

    bool EngineOperations::ProcessEvents() {
        return Events::Handler::PollEvents(mEventBus);
    }

    bool EngineOperations::PrepareSimulation() {
        return true;
    }

    void EngineOperations::ProcessInputs() {
    }

    void EngineOperations::UpdateSimulation(const float deltaTime) {
        mModules.camerasManager->Update(deltaTime);
    }

    void EngineOperations::Interpolate(float /*alpha*/) {

    }

    void EngineOperations::Render() {
        mModules.renderer->Draw();
    }

    Runtime::FrameOperations::Duration EngineOperations::GetFramePeriod() {
        return  mModules.renderer->GetFramePeriod();
    }
}

