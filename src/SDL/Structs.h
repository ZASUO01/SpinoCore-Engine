// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>

struct SDL_Window;
struct SDL_Surface;
struct TTF_Font;

namespace SpinoCore::SDL {
    struct SDLWindowDeleter {
        void operator()(SDL_Window* window) const;
    };
    using UniqueSDLWindow = std::unique_ptr<SDL_Window, SDLWindowDeleter>;

    struct GLContextDeleter {
        void operator()(void*) const;
    };
    using UniqueGLContext = std::unique_ptr<void, GLContextDeleter>;

    struct SDLSurfaceDeleter {
        void operator()(SDL_Surface* surface) const;
    };
    using UniqueSDLSurface = std::unique_ptr<SDL_Surface, SDLSurfaceDeleter>;

    struct TTFFontDeleter {
        void operator()(TTF_Font* font) const;
    };
    using UniqueTTFFont = std::unique_ptr<TTF_Font, TTFFontDeleter>;
}