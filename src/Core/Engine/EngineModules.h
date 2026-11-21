// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <optional>
#include "Core/Events/EventBus.h"
#include "Core/Window/EngineWindow.h"
#include "Core/Scenes/EngineScenesManager.h"
#include "Core/Rendering/Renderer.h"
#include "Core/Cameras/EngineCamerasManager.h"
#include "Core/UI/EngineScreensManager.h"
#include "Files/EngineFileManager.h"

namespace SpinoCore::Core::Engine {
    struct EngineModules final {
        Events::EventBus eventBus{};
        std::optional<Files::EngineFileManager> fileManager{};
        std::optional<Window::EngineWindow> window{};
        std::optional<Rendering::Renderer> renderer{};
        std::optional<Cameras::EngineCamerasManager> camerasManager{};
        UI::EngineScreensManager screensManager{};
        std::optional<Scenes::EngineScenesManager> scenesManager{};
    };
}
