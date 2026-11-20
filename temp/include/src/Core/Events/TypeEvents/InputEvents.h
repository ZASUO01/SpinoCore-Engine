// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "TypeEvent.h"
#include <SDL3/SDL.h>

namespace SpinoCore::Core::Events {
    struct KeyInputEvent final : TypeEvent {
        SDL_Event event{};
    };
}
