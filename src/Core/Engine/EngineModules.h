// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <optional>
#include "Core/Window/EngineWindow.h"
#include "Core/Scenes/EngineScenesManager.h"
#include "Core/Rendering/Renderer.h"

namespace SpinoCore::Core::Engine {
    struct EngineModules final {
        Events::EventBus eventBus{};
        std::optional<Window::EngineWindow> window{};
        std::optional<Rendering::Renderer> renderer{};
        std::optional<Scenes::EngineScenesManager> scenesManager{};
    };
}
