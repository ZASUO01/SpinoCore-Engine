// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <optional>
#include <gtest/gtest.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>
#include "GLTestContext.h"
#include "Core/Rendering/Drawer/DrawCommands.h"
#include "Core/Rendering/Drawer/Drawer.h"
#include "Core/Rendering/Renderer.h"
#include "Graphics/PostProcess/EnginePostProcessManager.h"
#include "Graphics/Resources/Texture.h"
#include "SpinoCore/Graphics/PostProcess.h"
#include "SpinoCore/Math/MatrixFactory.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Core::Rendering::Drawer;
    using SpinoCore::Core::Rendering::UIDrawCommand;
    using SpinoCore::Graphics::EnginePostProcessManager;
    using SpinoCore::Graphics::PostProcess;
    using SpinoCore::Graphics::PostProcessUniform;
    using SpinoCore::Graphics::UniformType;
    using SpinoCore::Graphics::Resources::Texture;
    using SpinoCore::Math::Vector2f;
    using SpinoCore::Math::Vector4f;
    using SpinoCore::Tests::GLTestContext;
    using SpinoCore::Tests::Pixel;
    namespace DefaultPostProcess = SpinoCore::Graphics::DefaultPostProcess;
    namespace MatrixFactory = SpinoCore::Math::MatrixFactory;

    constexpr Pixel Orange{200, 60, 20, 255};

    class PostProcessTests : public GLTestContext {
    protected:
        void SetUp() override {
            GLTestContext::SetUp();
            if (HasFatalFailure()) return;
            drawer.emplace();
            manager.emplace();
            SDL_Surface* surface = SDL_CreateSurface(1, 1, SDL_PIXELFORMAT_RGBA32);
            ASSERT_NE(surface, nullptr);
            SDL_FillSurfaceRect(surface, nullptr, SDL_MapSurfaceRGBA(surface, Orange[0], Orange[1], Orange[2], Orange[3]));
            scene.emplace(surface);
            SDL_DestroySurface(surface);
        }

        void TearDown() override {
            scene.reset();
            manager.reset();
            drawer.reset();
            GLTestContext::TearDown();
        }

        [[nodiscard]] Pixel FilteredCenter() {
            const auto& shader = manager->PrepareCurrentShader();
            drawer->DrawPostProcess(shader, scene->GetID(), Vector2f{16.0f, 16.0f}, 0.0f, manager->GetCurrentUniforms());
            return ReadPixel(8, 8);
        }

        [[nodiscard]] static PostProcess Tint() {
            return PostProcess{
                .fragmentShader = "layout (location = 0) uniform vec4 uTint; void main() { outColor = uTint; }",
                .uniforms = {PostProcessUniform{.type = UniformType::Vec4, .value = Vector4f{0.0f, 1.0f, 0.0f, 1.0f}}},
            };
        }

        std::optional<Drawer> drawer{};
        std::optional<EnginePostProcessManager> manager{};
        std::optional<Texture> scene{};
    };

    TEST_F(PostProcessTests, WithoutACurrentPostProcessTheSceneIsCopiedUnchanged) {
        const Pixel pixel = FilteredCenter();

        EXPECT_EQ(pixel, Orange);
    }

    TEST_F(PostProcessTests, GrayscaleMakesEveryChannelEqual) {
        manager->SetCurrent(DefaultPostProcess::Grayscale);

        const Pixel pixel = FilteredCenter();

        EXPECT_EQ(pixel[0], pixel[1]);
        EXPECT_EQ(pixel[1], pixel[2]);
    }

    TEST_F(PostProcessTests, ExposureWithNeutralValuesKeepsTheScene) {
        manager->SetCurrent(DefaultPostProcess::Exposure);

        const Pixel pixel = FilteredCenter();

        EXPECT_EQ(pixel, Orange);
    }

    TEST_F(PostProcessTests, AUniformIsAppliedAtTheLocationOfItsIndex) {
        manager->AddPostProcess("Tint", Tint());
        manager->SetCurrent("Tint");

        manager->SetUniform("Tint", 0, Vector4f{0.0f, 0.0f, 1.0f, 1.0f});

        EXPECT_EQ(FilteredCenter(), (Pixel{0, 0, 255, 255}));
    }

    TEST_F(PostProcessTests, TheResolutionReachesTheShader) {
        manager->AddPostProcess("Resolution", PostProcess{.fragmentShader = "void main() { outColor = vec4(uResolution.x / 16.0, 0.0, 0.0, 1.0); }", .uniforms = {}});
        manager->SetCurrent("Resolution");

        const Pixel pixel = FilteredCenter();

        EXPECT_EQ(pixel, (Pixel{255, 0, 0, 255}));
    }

    TEST_F(PostProcessTests, AShaderThatDoesNotCompileFallsBackToPassthrough) {
        manager->AddPostProcess("Broken", PostProcess{.fragmentShader = "void main() { this is not glsl }", .uniforms = {}});
        manager->SetCurrent("Broken");

        const Pixel pixel = FilteredCenter();

        EXPECT_EQ(pixel, Orange);
    }

    TEST_F(PostProcessTests, TheRendererFiltersTheWholeFrameWhenAPostProcessIsActive) {
        renderer->SetPostProcessManager(&*manager);
        manager->SetCurrent(DefaultPostProcess::Grayscale);
        manager->ToggleActive();
        UIDrawCommand background{};
        background.transform = MatrixFactory::CreateScale(10000.0f, 10000.0f, 1.0f);
        background.color = Vector4f{1.0f, 0.0f, 0.0f, 1.0f};
        background.size = {10000.0f, 10000.0f};
        renderer->GetUIDrawList().commands.push_back(background);

        renderer->Draw();

        const Pixel pixel = ReadPixel(8, 8);
        EXPECT_EQ(pixel[0], pixel[1]);
        EXPECT_GT(pixel[0], 0);
    }

    TEST_F(PostProcessTests, TheRendererSkipsTheFilterBufferWhenNothingIsActive) {
        renderer->SetPostProcessManager(&*manager);

        renderer->Draw();

        EXPECT_EQ(renderer->GetFilterBuffer(), nullptr);
    }
}
