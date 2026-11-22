// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>
#include <gtest/gtest.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>
#include <glad/glad.h>
#include "GLTestContext.h"
#include "Assets/AssetCache.h"
#include "Core/Rendering/Drawer/DrawCommands.h"
#include "Core/Rendering/Drawer/Drawer.h"
#include "Graphics/Geometry/VertexArray.h"
#include "Graphics/Resources/Model.h"
#include "Graphics/Resources/ModelData.h"
#include "Graphics/Resources/Texture.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/MatrixFactory.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Assets::AssetCache;
    using SpinoCore::Core::Rendering::Drawer;
    using SpinoCore::Core::Rendering::ModelDrawCommand;
    using SpinoCore::Graphics::Resources::Bone;
    using SpinoCore::Graphics::Resources::MaxBones;
    using SpinoCore::Graphics::Resources::MeshData;
    using SpinoCore::Graphics::Resources::Model;
    using SpinoCore::Graphics::Resources::ModelData;
    using SpinoCore::Graphics::Resources::ModelVertex;
    using SpinoCore::Graphics::Resources::Texture;
    using SpinoCore::Math::Matrix4f;
    using SpinoCore::Math::Vector3f;
    using SpinoCore::Math::Vector4f;
    using SpinoCore::Tests::GLTestContext;
    using SpinoCore::Tests::Pixel;
    namespace MatrixFactory = SpinoCore::Math::MatrixFactory;

    constexpr Pixel Empty{0, 0, 0, 0};
    constexpr GLint Center = 8;

    [[nodiscard]] MeshData Quad(const bool skinned) {
        MeshData quad{};
        for (const auto& [x, y] : std::vector<std::pair<float, float>>{{-0.5f, -0.5f}, {0.5f, -0.5f}, {0.5f, 0.5f}, {-0.5f, 0.5f}}) {
            ModelVertex vertex{};
            vertex.position = {x, y, 0.0f};
            vertex.normal = {0.0f, 0.0f, 1.0f};
            vertex.uv = {x + 0.5f, y + 0.5f};
            vertex.weights = {1.0f, 0.0f, 0.0f, 0.0f};
            quad.vertices.push_back(vertex);
        }
        quad.indices = {0, 1, 2, 2, 3, 0};
        quad.skinned = skinned;
        return quad;
    }

    [[nodiscard]] ModelData QuadModel(const bool skinned) {
        ModelData data{};
        data.meshes.push_back(Quad(skinned));
        if (skinned) {
            data.skeleton.bones.push_back(Bone{.name = "root"});
            data.skeleton.evaluationOrder = {0};
        }
        return data;
    }

    class ModelTests : public GLTestContext {
    protected:
        void SetUp() override {
            GLTestContext::SetUp();
            if (HasFatalFailure()) return;
            drawer.emplace();
            textures.emplace(std::make_unique<const Texture>());
        }

        void TearDown() override {
            drawer.reset();
            textures.reset();
            GLTestContext::TearDown();
        }

        void Draw(const Model& model, const std::vector<Matrix4f>& bones = {}) {
            drawer->GetModelDrawList().textures = &*textures;
            drawer->GetModelDrawList().commands.push_back(ModelDrawCommand{.key = 0, .model = &model, .world = Matrix4f::Identity(), .bones = bones});
            drawer->DrawModels(Matrix4f::Identity());
        }

        std::optional<Drawer> drawer{};
        std::optional<AssetCache<Texture>> textures{};
    };

    TEST_F(ModelTests, TheDefaultModelIsASingleCubeMesh) {
        const Model cube;

        EXPECT_EQ(cube.GetMeshes().size(), 1u);
        EXPECT_EQ(cube.GetMeshes()[0].vertices->GetIndexCount(), 36u);
    }

    TEST_F(ModelTests, AMeshUsingAMissingMaterialIsRejected) {
        const ModelData data = QuadModel(false);

        EXPECT_THROW((Model{data, {}}), std::invalid_argument);
    }

    TEST_F(ModelTests, ASkeletonLargerThanTheShaderLimitIsRejected) {
        ModelData data = QuadModel(true);
        data.skeleton.bones.resize(MaxBones + 1);

        EXPECT_THROW((Model{data, {Model::Material{}}}), std::invalid_argument);
    }

    TEST_F(ModelTests, ClipsAreFoundByName) {
        ModelData data = QuadModel(true);
        data.clips.push_back({.name = "run", .duration = 1.0f, .tracks = {}});
        const Model model{data, {Model::Material{}}};

        EXPECT_EQ(model.FindClip("run"), &model.GetClips()[0]);
        EXPECT_EQ(model.FindClip("fly"), nullptr);
    }

    TEST_F(ModelTests, ModelsAreDrawnWithTheirMaterialColor) {
        const Model model{QuadModel(false), {Model::Material{.baseColor = Vector4f{1.0f, 0.0f, 0.0f, 1.0f}, .textureId = {}}}};

        Draw(model);

        const Pixel pixel = ReadPixel(Center, Center);
        EXPECT_GT(pixel[0], 0);
        EXPECT_EQ(pixel[1], 0);
        EXPECT_EQ(pixel[2], 0);
    }

    TEST_F(ModelTests, ModelsAreTintedByTheirMaterialTexture) {
        SDL_Surface* surface = SDL_CreateSurface(1, 1, SDL_PIXELFORMAT_RGBA32);
        ASSERT_NE(surface, nullptr);
        SDL_FillSurfaceRect(surface, nullptr, SDL_MapSurfaceRGBA(surface, 0, 255, 0, 255));
        textures->Add("grass", std::make_unique<const Texture>(surface));
        SDL_DestroySurface(surface);
        const Model model{QuadModel(false), {Model::Material{.baseColor = Vector4f::One(), .textureId = "grass"}}};

        Draw(model);

        const Pixel pixel = ReadPixel(Center, Center);
        EXPECT_EQ(pixel[0], 0);
        EXPECT_GT(pixel[1], 0);
    }

    TEST_F(ModelTests, SkinnedMeshesFollowTheirBones) {
        const Model model{QuadModel(true), {Model::Material{}}};
        const std::vector<Matrix4f> bones{MatrixFactory::CreateTranslation(Vector3f{10.0f, 0.0f, 0.0f})};

        Draw(model, bones);

        EXPECT_EQ(ReadPixel(Center, Center), Empty);
    }

    TEST_F(ModelTests, SkinnedMeshesWithoutBonesAreDrawnInTheirBindPose) {
        const Model model{QuadModel(true), {Model::Material{}}};

        Draw(model);

        EXPECT_NE(ReadPixel(Center, Center), Empty);
    }

    TEST_F(ModelTests, DrawingConsumesTheModelCommands) {
        const Model model{QuadModel(false), {Model::Material{}}};

        Draw(model);

        EXPECT_TRUE(drawer->GetModelDrawList().commands.empty());
    }
}
