// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <optional>
#include "Core/Scenes/EngineScenesManager.h"

namespace SpinoCore::Core::Engine {
    struct EngineModules final {
        std::optional<Scenes::EngineScenesManager> scenesManager{};
    };
}
