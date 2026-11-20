// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Handler.h"
#include <SDL3/SDL_events.h>
#include "EventBus.h"
#include "TypeEvents/WindowEvents.h"

namespace SpinoCore::Core::Events {
    namespace {
        void DispatchResize(EventBus& bus, const SDL_WindowID windowID) {
            SDL_Window* window = SDL_GetWindowFromID(windowID);
            if (!window) return;

            WindowResizeEvent resizeEvent{};
            SDL_GetWindowSize(window, &resizeEvent.size.width, &resizeEvent.size.height);
            SDL_GetWindowSizeInPixels(window, &resizeEvent.pixelSize.width, &resizeEvent.pixelSize.height);
            bus.Dispatch(resizeEvent);
        }

        void DispatchVisibility(EventBus& bus, const bool visible) {
            WindowVisibilityChangedEvent event;
            event.visible = visible;
            bus.Dispatch(event);
        }
    }

    bool Handler::PollEvents(EventBus& bus) {
        bool keepRunning = true;
        SDL_WindowID resizedWindow = 0;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                case SDL_EVENT_TERMINATING:
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    keepRunning = false;
                    break;
                case SDL_EVENT_WINDOW_RESIZED:
                case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
                    resizedWindow = event.window.windowID;
                    break;
                case SDL_EVENT_WINDOW_MINIMIZED:
                case SDL_EVENT_WINDOW_HIDDEN:
                case SDL_EVENT_WINDOW_OCCLUDED:
                    DispatchVisibility(bus, false);
                    break;
                case SDL_EVENT_WINDOW_RESTORED:
                case SDL_EVENT_WINDOW_SHOWN:
                case SDL_EVENT_WINDOW_EXPOSED:
                    DispatchVisibility(bus, true);
                    break;
                case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
                case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
                case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
                case SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED: {
                    WindowPresentationChangedEvent presentationChangedEvent{};
                    bus.Dispatch(presentationChangedEvent);
                    break;
                }
                default:
                    break;
            }
        }
        if (resizedWindow != 0) DispatchResize(bus, resizedWindow);
        return keepRunning;
    }
}
