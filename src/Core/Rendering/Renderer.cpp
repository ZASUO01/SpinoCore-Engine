// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <SDL3/SDL.h>
#include <glad/glad.h>
#include "Renderer.h"
#include "SpinoCore/Math/MatrixFactory.h"

namespace SpinoCore::Core::Rendering {
    Renderer::Renderer(SDL_Window *window, const Utils::Size2D windowSize, Events::EventBus& bus)
     :mPresenter(window, bus)
    {
        Utils::Size2D pixelSize{};
        SDL_GetWindowSizeInPixels(window, &pixelSize.width, &pixelSize.height);
        Resize(windowSize, pixelSize);

        glClearColor(0.54f, 0.64f, 0.72f, 1.0f);

        mResizeSubscription = bus.Listen<Events::WindowResizeEvent, &Renderer::OnResize>(this);
    }

    Renderer::~Renderer() = default;

    void Renderer::Draw() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        mPresenter.Present();
    }

    void Renderer::SetCamera(const Math::Matrix4f& view, const Math::Matrix4f& projection) {
        mCamera.view = view;
        mCamera.projection = projection;
        mCamera.viewProjection = projection * view;
    }

    void Renderer::OnResize(const Events::WindowResizeEvent& event) {
        Resize(event.size, event.pixelSize);
    }

    void Renderer::Resize(const Utils::Size2D size, const Utils::Size2D pixelSize) {
        if (size.width <= 0 || size.height <= 0 || pixelSize.width <= 0 || pixelSize.height <= 0) return;

        glViewport(0, 0, pixelSize.width, pixelSize.height);
        mOrthoViewProjectionMatrix = Math::MatrixFactory::CreateOrtho(
            0.0f, static_cast<float>(size.width), static_cast<float>(size.height), 0.0f,
            ORTHO_NEAR_PLANE, ORTHO_FAR_PLANE);
    }
}

