// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Renderer.h"
#include "glad/glad.h"
#include "SDL/Helpers/Helpers.h"
#include <SDL3/SDL_video.h>

namespace SpinoCore::Core::Rendering {
    namespace {
        constexpr int VSyncOn = 1;
        constexpr int VSyncOff = 0;
    }

    Renderer::Renderer(SDL_Window *window) {
        auto glContext = SDL::Helpers::CreateGLContext(window);
        if (!glContext) throw std::runtime_error("[RENDERER] Failed to create OpenGL context");

        mSDLWindow = window;
        mGLContext = std::move(glContext);

        if (!SDL_GL_MakeCurrent(window, static_cast<SDL_GLContext>(mGLContext.get()))) {
            throw std::runtime_error("[RENDERER] Failed to make current GL context");
        }


        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
            throw std::runtime_error("[RENDERER] Failed to init GLAD.");
        }

        SDL_GL_SetSwapInterval(VSyncOn);
        glClearColor(0.54f, 0.64f, 0.72f, 1.0f);
    }

    Renderer::~Renderer() = default;

    void Renderer::Draw() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        SDL_GL_SwapWindow(mSDLWindow);
    }
}
