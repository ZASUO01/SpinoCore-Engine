// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Handler.h"
#include <SDL3/SDL_events.h>

namespace SpinoCore::Core::Events::Handler {
    bool PollEvents() noexcept {
        bool keepRunning = true;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                case SDL_EVENT_TERMINATING:
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    keepRunning = false;
                    break;
                default:
                    break;
            }
        }

        return keepRunning;
    }
}