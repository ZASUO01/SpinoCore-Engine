// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <chrono>
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

namespace SpinoCore::Graphics {
    class EnginePostProcessManager;
}

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
        void SetPostProcessManager(Graphics::EnginePostProcessManager* postProcessManager) noexcept { mPostProcess = postProcessManager; }
        [[nodiscard]] const Graphics::FrameBuffer* GetFilterBuffer() const noexcept { return mFilterBuffer ? &*mFilterBuffer : nullptr; }
        [[nodiscard]] const Graphics::FrameBuffer* GetShadowMap() const noexcept { return mShadowMap ? &*mShadowMap : nullptr; }

    private:
        static constexpr float ORTHO_NEAR_PLANE = -1.0f;
        static constexpr float ORTHO_FAR_PLANE = 1.0f;
        static constexpr int SHADOW_MAP_SIZE = 2048;

        void DrawShadowMap();
        void DrawScene();
        void BeginPostProcess();
        void EndPostProcess();

        void OnResize(const Events::WindowResizeEvent& event);
        void Resize(Utils::Size2D size, Utils::Size2D pixelSize);

        Presenter mPresenter;
        Drawer mDrawer;
        Lighting::GlobalLight mGlobalLight;
        std::optional<Graphics::FrameBuffer> mShadowMap{};
        Graphics::EnginePostProcessManager* mPostProcess{nullptr};
        std::optional<Graphics::FrameBuffer> mFilterBuffer{};
        Utils::Size2D mPixelSize{};
        std::chrono::steady_clock::time_point mStartTime{std::chrono::steady_clock::now()};
        CameraMatrices mCamera{};
        Math::Matrix4f mOrthoViewProjectionMatrix{Math::Matrix4f::Identity()};
        Events::EventBus::Subscription mResizeSubscription;
    };
}
