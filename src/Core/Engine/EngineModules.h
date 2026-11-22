// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <optional>
#include "Core/Events/EventBus.h"
#include "SpinoCore/Core/Input/InputState.h"
#include "Core/Input/SimulationInput.h"
#include "Core/Actors/EngineActorsManager.h"
#include "Core/Window/EngineWindow.h"
#include "Core/Scenes/EngineScenesManager.h"
#include "Core/Rendering/Renderer.h"
#include "Core/Cameras/EngineCamerasManager.h"
#include "Core/UI/EngineScreensManager.h"
#include "Files/EngineFileManager.h"
#include "Graphics/PostProcess/EnginePostProcessManager.h"
#include "Physics/CollisionProcessor.h"

namespace SpinoCore::Core::Engine {
    struct EngineModules final {
        Events::EventBus eventBus{};
        Input::InputState input{};
        Input::SimulationInput simulationInput{};
        std::optional<Files::EngineFileManager> fileManager{};
        std::optional<Window::EngineWindow> window{};
        std::optional<Rendering::Renderer> renderer{};
        Graphics::EnginePostProcessManager postProcessManager{};
        std::optional<Cameras::EngineCamerasManager> camerasManager{};
        UI::EngineScreensManager screensManager{};
        Physics::CollisionProcessor collisionProcessor{};
        Actors::EngineActorsManager actorsManager{&collisionProcessor};
        std::optional<Scenes::EngineScenesManager> scenesManager{};
    };
}
