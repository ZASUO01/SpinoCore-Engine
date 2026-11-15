// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Renderer.h"
#include "glad/glad.h"
#include "SDL/Helpers/Helpers.h"
#include <SDL3/SDL_video.h>

#include "Core/Events/TypeEvents/WindowEvents.h"
#include "SpinoCore/Math/MatrixFactory.h"

namespace SpinoCore::Core::Rendering {
    namespace {
        constexpr int VSyncOn = 1;
        constexpr int VSyncOff = 0;
    }

    Renderer::Renderer(SDL_Window *window, const Utils::Size2D windowSize, Events::EventBus& bus) {
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

        Utils::Size2D pixelSize{};
        SDL_GetWindowSizeInPixels(window, &pixelSize.width, &pixelSize.height);
        Resize(windowSize, pixelSize);

        mResizeSubscription = bus.Listen<Events::WindowResizeEvent>([](Events::WindowResizeEvent& event, void* context) {
            static_cast<Renderer*>(context)->Resize(event.size, event.pixelSize);
        }, this);

        mMinimizeSubscription = bus.Listen<Events::WindowMinimizeEvent>([](Events::WindowMinimizeEvent& event, void* context) {
            static_cast<Renderer*>(context)->mMinimized = event.minimized;
        }, this);
    }

    Renderer::~Renderer() = default;

    void Renderer::Draw() {
        if (mMinimized) return;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        SDL_GL_SwapWindow(mSDLWindow);
    }

    void Renderer::SetProjectionMatrix(const Math::Matrix4f &matrix) {
        mProjectionMatrix = matrix;
        mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
    }

    void Renderer::SetViewMatrix(const Math::Matrix4f &matrix) {
        mViewMatrix = matrix;
        mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
    }

    void Renderer::Resize(const Utils::Size2D size, const Utils::Size2D pixelSize) {
        if (size.width <= 0 || size.height <= 0 || pixelSize.width <= 0 || pixelSize.height <= 0) return;

        glViewport(0, 0, pixelSize.width, pixelSize.height);
        SetOrthoViewMatrix(static_cast<float>(size.width), static_cast<float>(size.height), ORTHO_NEAR_PLANE, ORTHO_FAR_PLANE);
    }

    void Renderer::SetOrthoViewMatrix(const float width, const float height, const float near, const float far) {
        mOrthoViewProjectionMatrix = Math::MatrixFactory::CreateOrtho(0.0f, width, height, 0.0f, near, far);
    }
}
