// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>

struct SDL_Window;

namespace SpinoCore::SDL {
    struct SDLWindowDeleter {
        void operator()(SDL_Window* window) const;
    };
    using UniqueSDLWindow = std::unique_ptr<SDL_Window, SDLWindowDeleter>;

    struct GLContextDeleter {
        void operator()(void*) const;
    };
    using UniqueGLContext = std::unique_ptr<void, GLContextDeleter>;
}