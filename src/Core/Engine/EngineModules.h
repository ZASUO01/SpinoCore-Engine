// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <optional>
#include "Core/Scenes/EngineScenesManager.h"
#include "Core/Window/EngineWindow.h"

namespace SpinoCore::Core::Engine {
    struct EngineModules final {
        std::optional<Window::EngineWindow> window{};
        std::optional<Scenes::EngineScenesManager> scenesManager{};
    };
}
