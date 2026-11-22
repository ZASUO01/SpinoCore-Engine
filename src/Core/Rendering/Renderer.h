// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <optional>
#include "SpinoCore/Utils/Size2D.h"
#include "SDL/Structs.h"
#include "Core/Events/EventBus.h"
#include "Core/Events/TypeEvents/WindowEvents.h"
#include "Core/Rendering/Presenter/Presenter.h"
#include "Drawer/DrawCommands.h"
#include "Drawer/Drawer.h"
#include "SpinoCore/Math/Matrix.h"
#include "Graphics/FrameBuffer.h"
#include "SpinoCore/Core/Lighting/GlobalLight.h"

namespace SpinoCore::Core::Rendering {
    struct CameraMatrices final {
        Math::Matrix4f view{Math::Matrix4f::Identity()};
        Math::Matrix4f projection{Math::Matrix4f::Identity()};
        Math::Matrix4f viewProjection{Math::Matrix4f::Identity()};
    };

    class Renderer final {
    public:
        using Duration = Presenter::Duration;

        Renderer(SDL_Window* window, Utils::Size2D windowSize, Events::EventBus& bus,
                 const Lighting::GlobalLight& globalLight = Lighting::GlobalLight{});
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
        [[nodiscard]] ModelDrawList& GetModelDrawList() { return mDrawer.GetModelDrawList(); }

        [[nodiscard]] Lighting::GlobalLight& GetGlobalLight() noexcept { return mGlobalLight; }
        [[nodiscard]] const Graphics::FrameBuffer* GetShadowMap() const noexcept { return mShadowMap ? &*mShadowMap : nullptr; }

    private:
        static constexpr float ORTHO_NEAR_PLANE = -1.0f;
        static constexpr float ORTHO_FAR_PLANE = 1.0f;
        static constexpr int SHADOW_MAP_SIZE = 2048;

        void DrawShadowMap();

        void OnResize(const Events::WindowResizeEvent& event);
        void Resize(Utils::Size2D size, Utils::Size2D pixelSize);

        Presenter mPresenter;
        Drawer mDrawer;
        Lighting::GlobalLight mGlobalLight;
        std::optional<Graphics::FrameBuffer> mShadowMap{};
        CameraMatrices mCamera{};
        Math::Matrix4f mOrthoViewProjectionMatrix{Math::Matrix4f::Identity()};
        Events::EventBus::Subscription mResizeSubscription;
    };
}
