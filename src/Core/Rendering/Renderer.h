// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "SpinoCore/Utils/Size2D.h"
#include "SDL/Structs.h"
#include "Core/Events/EventBus.h"
#include "Core/Events/TypeEvents/WindowEvents.h"
#include "Core/Rendering/Presenter/Presenter.h"
#include "Drawer/DrawCommands.h"
#include "Drawer/Drawer.h"
#include "SpinoCore/Math/Matrix.h"

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
        [[nodiscard]] const CameraMatrices& GetCameraMatrices() const noexcept { return mCamera; }
        [[nodiscard]] const Math::Matrix4f& GetOrthoViewProjection() const noexcept { return mOrthoViewProjectionMatrix; }

        [[nodiscard]] UIDrawList& GetUIDrawList() { return mDrawer.GetUIDrawList(); }

    private:
        static constexpr float ORTHO_NEAR_PLANE = -1.0f;
        static constexpr float ORTHO_FAR_PLANE = 1.0f;

        void OnResize(const Events::WindowResizeEvent& event);
        void Resize(Utils::Size2D size, Utils::Size2D pixelSize);

        Presenter mPresenter;
        Drawer mDrawer;
        CameraMatrices mCamera{};
        Math::Matrix4f mOrthoViewProjectionMatrix{Math::Matrix4f::Identity()};
        Events::EventBus::Subscription mResizeSubscription;
    };
}
