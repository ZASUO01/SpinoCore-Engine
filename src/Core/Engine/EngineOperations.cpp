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
        mModules.fileManager->ProcessRequests();
        return mModules.fileManager->IsReady();
    }

    void EngineOperations::ProcessFrameInputs() {}

    void EngineOperations::ProcessSimulationInputs() {}

    void EngineOperations::UpdateSimulation(const float deltaTime) {
        mModules.camerasManager->Update(deltaTime);
    }

    void EngineOperations::Interpolate(float /*alpha*/) {}

    void EngineOperations::Render() {
        mModules.screensManager.GetDrawCommands(mModules.renderer->GetUIDrawList());
        mModules.renderer->Draw();
    }

    EngineOperations::Duration EngineOperations::GetFramePeriod() {
        return mModules.renderer->GetFramePeriod();
    }
}
