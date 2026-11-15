// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "Core/Cameras/EngineCamerasManager.h"
#include "Core/Rendering/Renderer.h"
#include "Core/Window/EngineWindow.h"

namespace SpinoCore::Core::Engine {
    struct EngineModules final {
        std::optional<Window::EngineWindow> window{};
        std::optional<Rendering::Renderer> renderer{};
        std::optional<Cameras::EngineCamerasManager> camerasManager{};
    };
}
