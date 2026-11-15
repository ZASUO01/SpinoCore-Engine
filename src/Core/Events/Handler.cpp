// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Handler.h"
#include <SDL3/SDL.h>
#include "TypeEvents/InputEvents.h"

namespace SpinoCore::Core::Events {
    bool Handler::PollEvents(EventBus& /*bus*/) {
        bool keepRunning = true;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                case SDL_EVENT_TERMINATING:
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    keepRunning = false;
                    break;
                case SDL_EVENT_KEY_DOWN:
                case SDL_EVENT_KEY_UP:
                case SDL_EVENT_MOUSE_BUTTON_UP:
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                case SDL_EVENT_MOUSE_MOTION: {

                    break;
                }
                default:
                    break;
            }
        }

        return keepRunning;
    }
}
