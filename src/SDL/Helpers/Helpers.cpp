// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SDL/Helpers/Helpers.h"
#include <string>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>
#include "SDL/Structs.h"

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

    UniqueSDLSurface CreateFromImage(const std::filesystem::path &path) noexcept {
        const std::u8string utf8 = path.u8string();
        const auto cpath = reinterpret_cast<const char*>(utf8.c_str());

        UniqueSDLSurface loaded{IMG_Load(cpath)};
        if (!loaded) return nullptr;

        if (loaded->format == SDL_PIXELFORMAT_RGBA32) return loaded;

        UniqueSDLSurface converted{SDL_ConvertSurface(loaded.get(), SDL_PIXELFORMAT_RGBA32)};
        if (!converted) return nullptr;

        return converted;
    }
}