// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <optional>
#include <utility>
#include <vector>
#include <gtest/gtest.h>
#include <glad/glad.h>
#include "../../Graphics/GLTestContext.h"
#include "Core/Rendering/Drawer/DrawCommands.h"
#include "Core/Rendering/Drawer/Drawer.h"
#include "Core/Rendering/Renderer.h"
#include "Graphics/FrameBuffer.h"
#include "Graphics/Resources/Model.h"
#include "Graphics/Resources/ModelData.h"
#include "Graphics/Resources/Texture.h"
#include "SpinoCore/Core/Lighting/GlobalLight.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/MatrixFactory.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Core::Lighting::GlobalLight;
    using SpinoCore::Core::Lighting::GlobalLightSettings;
    using SpinoCore::Core::Rendering::Drawer;
    using SpinoCore::Core::Rendering::ModelDrawCommand;
    using SpinoCore::Core::Rendering::ModelLighting;
    using SpinoCore::Core::Rendering::UIDrawCommand;
    using SpinoCore::Graphics::FrameBuffer;
    using SpinoCore::Graphics::FrameBufferFormat;
    using SpinoCore::Graphics::Resources::MeshData;
    using SpinoCore::Graphics::Resources::Model;
    using SpinoCore::Graphics::Resources::ModelData;
    using SpinoCore::Graphics::Resources::ModelVertex;
    using SpinoCore::Math::Matrix4f;
    using SpinoCore::Math::Vector3f;
    using SpinoCore::Math::Vector4f;
    using SpinoCore::Tests::GLTestContext;
    using SpinoCore::Tests::Pixel;
    namespace MatrixFactory = SpinoCore::Math::MatrixFactory;

    [[nodiscard]] ModelData Quad(const float left, const float right, const float bottom, const float top, const float depth) {
        MeshData quad{};
        for (const auto& [x, y] : std::vector<std::pair<float, float>>{{left, bottom}, {right, bottom}, {right, top}, {left, top}}) {
            ModelVertex vertex{};
            vertex.position = {x, y, depth};
            vertex.normal = {0.0f, 0.0f, 1.0f};
            quad.vertices.push_back(vertex);
        }
        quad.indices = {0, 1, 2, 2, 3, 0};

        ModelData data{};
        data.meshes.push_back(quad);
        return data;
    }

    [[nodiscard]] GlobalLight LightFacingTheFloor() {
        return GlobalLight{true, GlobalLightSettings{
            .position = Vector3f{0.0f, 0.0f, 5.0f},
            .target = Vector3f::Zero(),
            .color = Vector3f::One(),
            .intensity = 1.0f,
            .ambientColor = Vector3f{0.1f, 0.1f, 0.1f},
            .shadowExtent = 1.0f,
            .shadowNear = 0.1f,
            .shadowFar = 10.0f,
        }};
    }

    class GlobalLightRenderingTests : public GLTestContext {
    protected:
        void SetUp() override {
            GLTestContext::SetUp();
            if (HasFatalFailure()) return;
            drawer.emplace();
            floor.emplace(Quad(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f), std::vector{Model::Material{}});
            occluder.emplace(Quad(-1.0f, 0.0f, -1.0f, 1.0f, 1.0f), std::vector{Model::Material{}});
        }

        void TearDown() override {
            floor.reset();
            occluder.reset();
            drawer.reset();
            GLTestContext::TearDown();
        }

        void Push(const Model& model) {
            drawer->GetModelDrawList().commands.push_back(ModelDrawCommand{.key = 0, .model = &model, .world = Matrix4f::Identity(), .bones = {}});
        }

        void DrawWithShadows() {
            FrameBuffer shadowMap{{.width = 64, .height = 64, .format = FrameBufferFormat::DepthOnlyTexture}};
            Push(*occluder);
            Push(*floor);
            shadowMap.Bind();
            glClear(GL_DEPTH_BUFFER_BIT);
            drawer->DrawShadows(light.GetViewProjection());
            shadowMap.Unbind();

            drawer->GetModelDrawList().commands.clear();
            Push(*floor);
            const ModelLighting lighting{.light = light, .shadowMap = shadowMap.GetTexture()->GetID()};
            drawer->DrawModels(Matrix4f::Identity(), &lighting);
        }

        std::optional<Drawer> drawer{};
        std::optional<Model> floor{};
        std::optional<Model> occluder{};
        GlobalLight light = LightFacingTheFloor();
    };

    TEST_F(GlobalLightRenderingTests, AShadowedPixelIsDarkerThanALitOne) {
        DrawWithShadows();

        const Pixel shadowed = ReadPixel(4, 8);
        const Pixel lit = ReadPixel(12, 8);
        EXPECT_LT(shadowed[0], lit[0]);
    }

    TEST_F(GlobalLightRenderingTests, TheShadowPassLeavesTheTargetFrameBufferBound) {
        GLint before = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &before);

        DrawWithShadows();

        GLint after = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &after);
        EXPECT_EQ(after, before);
    }

    TEST_F(GlobalLightRenderingTests, ARendererWithoutTheGlobalLightHasNoShadowPass) {
        EXPECT_EQ(renderer->GetShadowMap(), nullptr);
    }

    TEST_F(GlobalLightRenderingTests, ARendererWithTheGlobalLightHasAShadowPass) {
        renderer.reset();

        renderer.emplace(window->GetSDLWindow(), window->GetSize(), bus, LightFacingTheFloor());

        EXPECT_NE(renderer->GetShadowMap(), nullptr);
    }

    TEST_F(GlobalLightRenderingTests, ARendererWithTheGlobalLightStillDrawsTheUI) {
        floor.reset();
        occluder.reset();
        drawer.reset();
        renderer.reset();
        renderer.emplace(window->GetSDLWindow(), window->GetSize(), bus, LightFacingTheFloor());
        const Model cube;
        FrameBuffer target{{.width = TargetSize, .height = TargetSize, .format = FrameBufferFormat::ColorWithDepthRBO}};
        target.Bind();
        renderer->GetModelDrawList().commands.push_back(ModelDrawCommand{.key = 0, .model = &cube, .world = Matrix4f::Identity(), .bones = {}});
        UIDrawCommand background{};
        background.transform = MatrixFactory::CreateScale(10000.0f, 10000.0f, 1.0f);
        background.color = Vector4f{0.0f, 0.0f, 1.0f, 1.0f};
        background.size = {10000.0f, 10000.0f};
        renderer->GetUIDrawList().commands.push_back(background);

        renderer->Draw();

        const Pixel center = ReadPixel(8, 8);
        target.Unbind();
        EXPECT_EQ(center, (Pixel{0, 0, 255, 255}));
    }
}
