// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <string>
#include <memory>

struct SDL_Window;

namespace SpinoCore::SDL {
    struct AppMetadata {
        std::string appName;
        std::string appVersion;
        std::string appIdentifier;
        std::string appCreator;
        std::string appURL;
    };

    struct SDLWindowData {
        std::string title;
        int width;
        int height;
    };

    struct SDLWindowDeleter {
        void operator()(SDL_Window* window) const;
    };
    using UniqueSDLWindow = std::unique_ptr<SDL_Window, SDLWindowDeleter>;

    struct GLContextDeleter {
        void operator()(void*) const;
    };
    using UniqueGLContext = std::unique_ptr<void, GLContextDeleter>;
}