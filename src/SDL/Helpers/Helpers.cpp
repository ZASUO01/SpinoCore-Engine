// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SDL/Helpers/Helpers.h"
#include <SDL3/SDL.h>

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
}

namespace SpinoCore::SDL::Helpers {
    UniqueSDLWindow CreateSDLWindow(const char *title, const int width, const int height) noexcept {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
#ifndef NDEBUG
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

        return UniqueSDLWindow(SDL_CreateWindow(title, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN));
    }

    UniqueGLContext CreateGLContext(SDL_Window* window) noexcept {
        return UniqueGLContext(SDL_GL_CreateContext(window));
    }
}