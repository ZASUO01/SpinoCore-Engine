// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Structs.h"
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <SDL3_ttf/SDL_ttf.h>

namespace SpinoCore::SDL {
    void SDLWindowDeleter::operator()(SDL_Window* window) const {
        if (window) {
            SDL_DestroyWindow(window);
        }
    }

    void GLContextDeleter::operator()(void* context) const {
        if (context) {
            SDL_GL_DestroyContext(static_cast<SDL_GLContext>(context));
        }
    }

    void SDLSurfaceDeleter::operator()(SDL_Surface *surface) const {
        if (surface) {
            SDL_DestroySurface(surface);
        }
    }

    void TTFFontDeleter::operator()(TTF_Font *font) const {
        if (font) {
            TTF_CloseFont(font);
        }
    }
}
