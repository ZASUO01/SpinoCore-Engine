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
#include "Sound/EngineSoundManager.h"
#include "Sound/Synth/EngineSynthManager.h"

namespace SpinoCore::Core::Engine {
    namespace {
        constexpr float SoundDuckingGain = 0.6f;
    }

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
        mModules.collisionProcessor.Update();
    }

    void EngineOperations::Interpolate(float /*alpha*/) {}

    void EngineOperations::Render() {
        mModules.synthManager->SetDucking(mModules.soundManager->IsAnyPlaying() ? SoundDuckingGain : 1.0f);
        mModules.actorsManager.GetDrawCommands(mModules.renderer->GetModelDrawList());
        mModules.actorsManager.GetParticleDrawCommands(mModules.renderer->GetParticleDrawList());
        if (mModules.renderer->IsDebugDrawEnabled()) mModules.actorsManager.GetDebugDrawCommands(mModules.renderer->GetDebugDrawList());
        mModules.screensManager.GetDrawCommands(mModules.renderer->GetUIDrawList());
        mModules.renderer->Draw();
    }

    EngineOperations::Duration EngineOperations::GetFramePeriod() {
        return mModules.renderer->GetFramePeriod();
    }
}
