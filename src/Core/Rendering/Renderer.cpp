// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <SDL3/SDL.h>
#include <glad/glad.h>
#include "Renderer.h"
#include "Graphics/PostProcess/EnginePostProcessManager.h"
#include "Graphics/Resources/Texture.h"
#include "SpinoCore/Math/Vector.h"
#include "SpinoCore/Math/MatrixFactory.h"

namespace SpinoCore::Core::Rendering {
    Renderer::Renderer(SDL_Window *window, const Utils::Size2D windowSize, Events::EventBus& bus, const Lighting::GlobalLight& globalLight,
                       const bool debugColliders)
     :mPresenter(window, bus)
     ,mGlobalLight(globalLight)
     ,mDebugColliders(debugColliders)
    {
        if (mGlobalLight.IsEnabled()) {
            mShadowMap.emplace(Graphics::FrameBufferSpecification{
                .width = SHADOW_MAP_SIZE, .height = SHADOW_MAP_SIZE, .format = Graphics::FrameBufferFormat::DepthOnlyTexture});
        }

        Utils::Size2D pixelSize{};
        SDL_GetWindowSizeInPixels(window, &pixelSize.width, &pixelSize.height);
        Resize(windowSize, pixelSize);

        glClearColor(0.54f, 0.64f, 0.72f, 1.0f);

        mResizeSubscription = bus.Listen<Events::WindowResizeEvent, &Renderer::OnResize>(this);
    }

    Renderer::~Renderer() = default;

    void Renderer::Draw() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (mShadowMap) DrawShadowMap();

        const bool filtered = mPostProcess && mPostProcess->IsActive();
        if (filtered) BeginPostProcess();
        DrawScene();
        if (filtered) EndPostProcess();

        mPresenter.Present();
    }

    void Renderer::DrawScene() {
        if (mShadowMap) {
            const ModelLighting lighting{.light = mGlobalLight, .shadowMap = mShadowMap->GetTexture()->GetID()};
            mDrawer.DrawModels(mCamera.viewProjection, &lighting);
        } else {
            mDrawer.DrawModels(mCamera.viewProjection);
        }
        mDrawer.DrawParticles(mCamera.view, mCamera.viewProjection);
        if (mDebugColliders) mDrawer.DrawDebug(mCamera.viewProjection);
        mDrawer.DrawUI(mOrthoViewProjectionMatrix);
    }

    void Renderer::BeginPostProcess() {
        if (!mFilterBuffer) {
            mFilterBuffer.emplace(Graphics::FrameBufferSpecification{
                .width = mPixelSize.width, .height = mPixelSize.height, .format = Graphics::FrameBufferFormat::ColorWithDepthRBO});
        }
        mFilterBuffer->Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::EndPostProcess() {
        mFilterBuffer->Unbind();

        const auto& shader = mPostProcess->PrepareCurrentShader();
        const auto elapsed = std::chrono::duration<float>(std::chrono::steady_clock::now() - mStartTime).count();
        const Math::Vector2f resolution{static_cast<float>(mPixelSize.width), static_cast<float>(mPixelSize.height)};
        mDrawer.DrawPostProcess(shader, mFilterBuffer->GetTexture()->GetID(), resolution, elapsed, mPostProcess->GetCurrentUniforms());
    }

    void Renderer::DrawShadowMap() {
        mShadowMap->Bind();
        glClear(GL_DEPTH_BUFFER_BIT);
        mDrawer.DrawShadows(mGlobalLight.GetViewProjection());
        mShadowMap->Unbind();
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
        mPixelSize = pixelSize;
        if (mFilterBuffer) mFilterBuffer->Resize(pixelSize.width, pixelSize.height);
        mOrthoViewProjectionMatrix = Math::MatrixFactory::CreateOrtho(
            0.0f, static_cast<float>(size.width), static_cast<float>(size.height), 0.0f,
            ORTHO_NEAR_PLANE, ORTHO_FAR_PLANE);
    }
}

