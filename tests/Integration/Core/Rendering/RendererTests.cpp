// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <array>
#include <cstddef>
#include <optional>
#include <gtest/gtest.h>
#include <SDL3/SDL_hints.h>
#include <glad/glad.h>
#include "Core/Events/EventBus.h"
#include "Core/Events/TypeEvents/WindowEvents.h"
#include "Core/Rendering/Renderer.h"
#include "Core/Window/EngineWindow.h"
#include "SDL/Context.h"
#include "SpinoCore/Core/Window/WindowProps.h"
#include "SpinoCore/Core/Window/WindowSize.h"
#include "SpinoCore/Game/GameInfo.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/MatrixFactory.h"
#include "SpinoCore/Math/Vector.h"
#include "SpinoCore/Utils/Size2D.h"

namespace {
    using SpinoCore::Core::Events::EventBus;
    using SpinoCore::Core::Events::WindowResizeEvent;
    using SpinoCore::Core::Rendering::Renderer;
    using SpinoCore::Core::Window::EngineWindow;
    using SpinoCore::Core::Window::WINDOW_SIZE_MIN;
    using SpinoCore::Core::Window::WindowProps;
    using SpinoCore::Game::GameInfo;
    using SpinoCore::Math::Matrix4f;
    using SpinoCore::Math::Vector3f;
    using SpinoCore::Math::Vector4f;
    namespace MatrixFactory = SpinoCore::Math::MatrixFactory;
    using SpinoCore::SDL::Context;
    using SpinoCore::Utils::Size2D;

    using Viewport = std::array<GLint, 4>;

    [[nodiscard]] Viewport CurrentViewport() {
        Viewport viewport{};
        glGetIntegerv(GL_VIEWPORT, viewport.data());
        return viewport;
    }

    class RendererTests : public testing::Test {
    protected:
        void SetUp() override {
            SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen");
            SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
            context.emplace(GameInfo{});
            window.emplace(WindowProps{.size = WINDOW_SIZE_MIN});
            renderer.emplace(window->GetSDLWindow(), window->GetSize(), bus);
        }

        void DispatchResize(const Size2D pixelSize) {
            WindowResizeEvent event{};
            event.size = pixelSize;
            event.pixelSize = pixelSize;
            bus.Dispatch(event);
        }

        std::optional<Context> context{};
        EventBus bus{};
        std::optional<EngineWindow> window{};
        std::optional<Renderer> renderer{};
    };

    TEST_F(RendererTests, StartsWithAViewportCoveringTheWindow) {
        EXPECT_EQ(CurrentViewport(), (Viewport{0, 0, WINDOW_SIZE_MIN.width, WINDOW_SIZE_MIN.height}));
    }

    TEST_F(RendererTests, ResizingTheWindowResizesTheViewport) {
        DispatchResize({.width = 640, .height = 480});

        EXPECT_EQ(CurrentViewport(), (Viewport{0, 0, 640, 480}));
    }

    TEST_F(RendererTests, IgnoresAResizeToAnEmptyWindow) {
        DispatchResize({.width = 0, .height = 0});

        EXPECT_EQ(CurrentViewport(), (Viewport{0, 0, WINDOW_SIZE_MIN.width, WINDOW_SIZE_MIN.height}));
    }

    TEST_F(RendererTests, SetCameraStoresTheMatricesAndCombinesThemProjectionAfterView) {
        const Matrix4f view = MatrixFactory::CreateTranslation(Vector3f{0.0f, 0.0f, -5.0f});
        const Matrix4f projection = MatrixFactory::CreateScale(2.0f, 3.0f, 1.0f);

        renderer->SetCamera(view, projection);

        const auto& matrices = renderer->GetCameraMatrices();

        const Vector4f result = matrices.viewProjection * Vector4f{1.0f, 1.0f, 0.0f, 1.0f};
        EXPECT_TRUE(Vector4f::NearlyEqual(result, Vector4f{2.0f, 3.0f, -5.0f, 1.0f}));
        EXPECT_FLOAT_EQ(matrices.view(2, 3), -5.0f);
        EXPECT_FLOAT_EQ(matrices.projection(1, 1), 3.0f);
    }

    TEST_F(RendererTests, TheOrthoProjectionMapsWindowCornersToTheScreenCorners) {
        const auto& ortho = renderer->GetOrthoViewProjection();
        const auto width = static_cast<float>(WINDOW_SIZE_MIN.width);
        const auto height = static_cast<float>(WINDOW_SIZE_MIN.height);

        EXPECT_TRUE(Vector3f::NearlyEqual(ortho.TransformPoint(Vector3f{0.0f, 0.0f, 0.0f}), Vector3f{-1.0f, 1.0f, 0.0f}));
        EXPECT_TRUE(Vector3f::NearlyEqual(ortho.TransformPoint(Vector3f{width, height, 0.0f}), Vector3f{1.0f, -1.0f, 0.0f}));
    }

    TEST_F(RendererTests, TheOrthoProjectionFollowsTheWindowSize) {
        DispatchResize({.width = 640, .height = 480});

        const auto& ortho = renderer->GetOrthoViewProjection();
        EXPECT_TRUE(Vector3f::NearlyEqual(ortho.TransformPoint(Vector3f{640.0f, 480.0f, 0.0f}), Vector3f{1.0f, -1.0f, 0.0f}));
        EXPECT_TRUE(Vector3f::NearlyEqual(ortho.TransformPoint(Vector3f{320.0f, 240.0f, 0.0f}), Vector3f{0.0f, 0.0f, 0.0f}));
    }

    TEST_F(RendererTests, TheOrthoProjectionIgnoresAResizeToAnEmptyWindow) {
        const Matrix4f before = renderer->GetOrthoViewProjection();

        DispatchResize({.width = 0, .height = 0});

        for (std::size_t i = 0; i < 16; ++i) EXPECT_FLOAT_EQ(renderer->GetOrthoViewProjection().data[i], before.data[i]);
    }

    TEST_F(RendererTests, DrawConsumesTheUIDrawList) {
        renderer->GetUIDrawList().commands.push_back({.key = 1});
        renderer->GetUIDrawList().commands.push_back({.key = 2});

        renderer->Draw();

        EXPECT_TRUE(renderer->GetUIDrawList().commands.empty());
    }
}
