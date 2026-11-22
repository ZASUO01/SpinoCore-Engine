// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "EngineOperations.h"
#include <optional>
#include "EngineModules.h"
#include "Core/Actors/EngineActorsManager.h"
#include "Core/Cameras/EngineCamerasManager.h"
#include "Core/Events/Handler.h"
#include "Core/Input/SimulationInput.h"
#include "Core/Rendering/Renderer.h"
#include "Core/Scenes/EngineScenesManager.h"
#include "Core/UI/EngineScreensManager.h"
#include "Files/EngineFileManager.h"

namespace SpinoCore::Core::Engine {
    EngineOperations::EngineOperations(EngineModules &modules) :mModules(modules){}

    bool EngineOperations::ProcessEvents() {
        const bool keepRunning = Events::Handler::PollEvents(mModules.eventBus, mModules.input);
        mModules.simulationInput.BeginFrame(mModules.input);
        return keepRunning;
    }

    bool EngineOperations::PrepareSimulation() {
        mModules.fileManager->ProcessRequests();
        mModules.scenesManager->PrepareCurrentScene();
        return mModules.fileManager->IsReady();
    }

    void EngineOperations::ProcessFrameInputs() {
        mModules.screensManager.ProcessInputs(mModules.input);
    }

    void EngineOperations::ProcessSimulationInputs() {
        mModules.actorsManager.SendInputToActors(mModules.simulationInput.BeginTick());
    }

    void EngineOperations::DiscardSimulationInputs() {
        mModules.simulationInput.Discard();
    }

    void EngineOperations::UpdateSimulation(const float deltaTime) {
        mModules.scenesManager->UpdateCurrentScene();
        mModules.camerasManager->Update(deltaTime);
        mModules.actorsManager.UpdateActors(deltaTime);
    }

    void EngineOperations::Interpolate(float /*alpha*/) {}

    void EngineOperations::Render() {
        mModules.actorsManager.GetDrawCommands(mModules.renderer->GetModelDrawList());
        mModules.screensManager.GetDrawCommands(mModules.renderer->GetUIDrawList());
        mModules.renderer->Draw();
    }

    EngineOperations::Duration EngineOperations::GetFramePeriod() {
        return mModules.renderer->GetFramePeriod();
    }
}
