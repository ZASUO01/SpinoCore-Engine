// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "Presenter.h"
#include "RenderCommands.h"
#include "RendererOperations.h"
#include "Core/Events/EventBus.h"
#include "Core/Events/TypeEvents/WindowEvents.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Utils/Size2D.h"

namespace SpinoCore::Core::Rendering {
    struct CameraMatrices final {
        Math::Matrix4f view{Math::Matrix4f::Identity()};
        Math::Matrix4f projection{Math::Matrix4f::Identity()};
        Math::Matrix4f viewProjection{Math::Matrix4f::Identity()};
    };

    class Renderer final {
    public:
        using Duration = Presenter::Duration;

        explicit Renderer(SDL_Window* window, Utils::Size2D windowSize, Events::EventBus& bus);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        void Draw();
        [[nodiscard]] Duration GetFramePeriod() const noexcept { return mPresenter.GetFramePeriod(); }

        void SetCamera(const Math::Matrix4f& view, const Math::Matrix4f& projection);

        // UI elements push their commands here every frame; consumed by Draw().
        [[nodiscard]] UIDrawList& GetUIDrawList() noexcept { return mUICommands; }
    private:
        static constexpr float ORTHO_NEAR_PLANE = -1.0f;
        static constexpr float ORTHO_FAR_PLANE = 1.0f;

        void OnResize(const Events::WindowResizeEvent& event);
        void Resize(Utils::Size2D size, Utils::Size2D pixelSize);

        Presenter mPresenter;
        CameraMatrices mCamera{};
        Math::Matrix4f mOrthoViewProjectionMatrix{Math::Matrix4f::Identity()};
        RendererOperations mRendererOperations{};
        Events::EventBus::Subscription mResizeSubscription;

        UIDrawList mUICommands;
    };
}
