// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include "../../Support/GLTFBuilder.h"
#include "Graphics/Resources/ModelData.h"
#include "SpinoCore/Math/Math.h"
#include "SpinoCore/Math/MatrixFactory.h"
#include "SpinoCore/Math/Quaternion.h"
#include "SpinoCore/Math/Vector.h"
#include "Utils/ModelHelpers.h"

namespace {
    using SpinoCore::Graphics::Resources::Interpolation;
    using SpinoCore::Graphics::Resources::ModelData;
    using SpinoCore::Math::Quaternionf;
    using SpinoCore::Math::Vector3f;
    using SpinoCore::Math::Vector4f;
    using SpinoCore::Tests::GLTFBuilder;
    using SpinoCore::Utils::ParseModel;
    using Json = nlohmann::json;

    const std::filesystem::path ModelPath{"Models/test.gltf"};

    [[nodiscard]] std::vector<std::uint8_t> SkinnedTriangle() {
        GLTFBuilder builder;
        const auto positions = builder.AddFloats({0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f}, "VEC3", 3);
        const auto joints = builder.AddUnsignedBytes({0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0}, "VEC4", 3);
        const auto weights = builder.AddFloats({0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f}, "VEC4", 3);
        const auto indices = builder.AddUnsignedShorts({0, 1, 2});
        const auto inverseBinds = builder.AddFloats({
            1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, -1, 0, 1,
        }, "MAT4", 2);
        const auto rotationTimes = builder.AddFloats({0.0f, 1.0f}, "SCALAR", 2);
        const auto rotations = builder.AddFloats({0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7071068f, 0.7071068f}, "VEC4", 2);
        const auto translationTimes = builder.AddFloats({0.0f, 2.0f}, "SCALAR", 2);
        const auto translations = builder.AddFloats({0.0f, 0.0f, 0.0f, 3.0f, 0.0f, 0.0f}, "VEC3", 2);

        builder.gltf["nodes"] = Json::array({
            {{"name", "root"}, {"children", {1}}},
            {{"name", "tip"}, {"translation", {0.0, 1.0, 0.0}}},
            {{"name", "body"}, {"mesh", 0}, {"skin", 0}},
        });
        builder.gltf["scenes"] = Json::array({{{"nodes", {0, 2}}}});
        builder.gltf["scene"] = 0;
        builder.gltf["skins"] = Json::array({{{"joints", {0, 1}}, {"inverseBindMatrices", inverseBinds}}});
        builder.gltf["materials"] = Json::array({{{"pbrMetallicRoughness", {{"baseColorFactor", {1.0, 0.5, 0.25, 1.0}}}}}});
        builder.gltf["meshes"] = Json::array({{{"primitives", Json::array({{
            {"attributes", {{"POSITION", positions}, {"JOINTS_0", joints}, {"WEIGHTS_0", weights}}},
            {"indices", indices},
            {"material", 0},
        }})}}});
        builder.gltf["animations"] = Json::array({{
            {"name", "wave"},
            {"samplers", Json::array({
                {{"input", rotationTimes}, {"output", rotations}, {"interpolation", "LINEAR"}},
                {{"input", translationTimes}, {"output", translations}, {"interpolation", "STEP"}},
            })},
            {"channels", Json::array({
                {{"sampler", 0}, {"target", {{"node", 1}, {"path", "rotation"}}}},
                {{"sampler", 1}, {"target", {{"node", 0}, {"path", "translation"}}}},
            })},
        }});
        return builder.Build();
    }

    [[nodiscard]] std::vector<std::uint8_t> MovedStaticTriangle() {
        GLTFBuilder builder;
        const auto positions = builder.AddFloats({0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f}, "VEC3", 3);
        builder.gltf["nodes"] = Json::array({{{"mesh", 0}, {"translation", {5.0, 0.0, 0.0}}}});
        builder.gltf["meshes"] = Json::array({{{"primitives", Json::array({{{"attributes", {{"POSITION", positions}}}}})}}});
        return builder.Build();
    }

    [[nodiscard]] std::vector<std::uint8_t> TexturedTriangle(const std::string& imageUri) {
        GLTFBuilder builder;
        const auto positions = builder.AddFloats({0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f}, "VEC3", 3);
        builder.gltf["nodes"] = Json::array({{{"mesh", 0}}});
        builder.gltf["images"] = Json::array({{{"uri", imageUri}}});
        builder.gltf["textures"] = Json::array({{{"source", 0}}});
        builder.gltf["materials"] = Json::array({{{"pbrMetallicRoughness", {{"baseColorTexture", {{"index", 0}}}}}}});
        builder.gltf["meshes"] = Json::array({{{"primitives", Json::array({{{"attributes", {{"POSITION", positions}}}, {"material", 0}}})}}});
        return builder.Build();
    }

    [[nodiscard]] std::vector<std::uint8_t> CharacterWithAHat() {
        GLTFBuilder builder;
        const auto hatPositions = builder.AddFloats({0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f}, "VEC3", 3);
        const auto bodyPositions = builder.AddFloats({0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f}, "VEC3", 3);
        const auto joints = builder.AddUnsignedBytes({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, "VEC4", 3);
        const auto weights = builder.AddFloats({1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f}, "VEC4", 3);

        builder.gltf["nodes"] = Json::array({
            {{"name", "root"}, {"children", {1}}},
            {{"name", "head"}, {"translation", {0.0, 2.0, 0.0}}, {"children", {2}}},
            {{"name", "hat"}, {"translation", {0.0, 1.0, 0.0}}, {"mesh", 0}},
            {{"name", "body"}, {"mesh", 1}, {"skin", 1}},
        });
        builder.gltf["scenes"] = Json::array({{{"nodes", {0, 3}}}});
        builder.gltf["skins"] = Json::array({{{"joints", {0, 1}}}, {{"joints", {0, 1}}}});
        builder.gltf["meshes"] = Json::array({
            {{"primitives", Json::array({{{"attributes", {{"POSITION", hatPositions}}}}})}},
            {{"primitives", Json::array({{{"attributes", {{"POSITION", bodyPositions}, {"JOINTS_0", joints}, {"WEIGHTS_0", weights}}}}})}},
        });
        return builder.Build();
    }

    TEST(ModelHelpersTests, ReadsTheVertexPositions) {
        const auto file = SkinnedTriangle();

        const ModelData model = ParseModel(file, ModelPath);

        ASSERT_EQ(model.meshes.size(), 1u);
        EXPECT_TRUE(Vector3f::NearlyEqual(model.meshes[0].vertices[1].position, Vector3f{1.0f, 0.0f, 0.0f}));
    }

    TEST(ModelHelpersTests, ReadsTheTriangleIndices) {
        const auto file = SkinnedTriangle();

        const ModelData model = ParseModel(file, ModelPath);

        EXPECT_EQ(model.meshes[0].indices, (std::vector<std::uint32_t>{0, 1, 2}));
    }

    TEST(ModelHelpersTests, ReadsTheJointsOfEachVertex) {
        const auto file = SkinnedTriangle();

        const ModelData model = ParseModel(file, ModelPath);

        EXPECT_EQ(model.meshes[0].vertices[0].joints, (std::array<std::uint8_t, 4>{0, 1, 0, 0}));
    }

    TEST(ModelHelpersTests, ReadsTheWeightsOfEachVertex) {
        const auto file = SkinnedTriangle();

        const ModelData model = ParseModel(file, ModelPath);

        EXPECT_TRUE(Vector4f::NearlyEqual(model.meshes[0].vertices[0].weights, Vector4f{0.5f, 0.5f, 0.0f, 0.0f}));
    }

    TEST(ModelHelpersTests, AMeshThatUsesTheSkinIsSkinned) {
        const auto file = SkinnedTriangle();

        const ModelData model = ParseModel(file, ModelPath);

        EXPECT_TRUE(model.meshes[0].skinned);
    }

    TEST(ModelHelpersTests, ReadsOneBonePerSkinJoint) {
        const auto file = SkinnedTriangle();

        const ModelData model = ParseModel(file, ModelPath);

        ASSERT_EQ(model.skeleton.bones.size(), 2u);
        EXPECT_EQ(model.skeleton.bones[1].name, "tip");
    }

    TEST(ModelHelpersTests, ABoneKnowsItsParentBone) {
        const auto file = SkinnedTriangle();

        const ModelData model = ParseModel(file, ModelPath);

        EXPECT_EQ(model.skeleton.bones[1].parent, 0);
    }

    TEST(ModelHelpersTests, ABoneKeepsItsRestTranslation) {
        const auto file = SkinnedTriangle();

        const ModelData model = ParseModel(file, ModelPath);

        EXPECT_TRUE(Vector3f::NearlyEqual(model.skeleton.bones[1].translation, Vector3f{0.0f, 1.0f, 0.0f}));
    }

    TEST(ModelHelpersTests, ReadsTheInverseBindMatrices) {
        const auto file = SkinnedTriangle();

        const ModelData model = ParseModel(file, ModelPath);

        const Vector3f translation = SpinoCore::Math::MatrixFactory::GetTranslation(model.skeleton.bones[1].inverseBind);
        EXPECT_TRUE(Vector3f::NearlyEqual(translation, Vector3f{0.0f, -1.0f, 0.0f}));
    }

    TEST(ModelHelpersTests, ParentsComeBeforeTheirChildrenWhenEvaluatingThePose) {
        const auto file = SkinnedTriangle();

        const ModelData model = ParseModel(file, ModelPath);

        EXPECT_EQ(model.skeleton.evaluationOrder, (std::vector<std::size_t>{0, 1}));
    }

    TEST(ModelHelpersTests, ReadsTheClipNameAndTheDurationOfItsLongestChannel) {
        const auto file = SkinnedTriangle();

        const ModelData model = ParseModel(file, ModelPath);

        ASSERT_EQ(model.clips.size(), 1u);
        EXPECT_EQ(model.clips[0].name, "wave");
        EXPECT_FLOAT_EQ(model.clips[0].duration, 2.0f);
    }

    TEST(ModelHelpersTests, ReadsRotationKeyframesAsQuaternions) {
        const auto file = SkinnedTriangle();

        const ModelData model = ParseModel(file, ModelPath);

        const auto& tip = model.clips[0].tracks[0];
        const Quaternionf quarterTurn{Vector3f::UnitZ(), SpinoCore::Math::Constants::PIOver2<float>};
        EXPECT_EQ(tip.bone, 1u);
        EXPECT_TRUE(Quaternionf::NearlyEqual(tip.rotation.values[1], quarterTurn));
    }

    TEST(ModelHelpersTests, KeepsTheInterpolationOfEachChannel) {
        const auto file = SkinnedTriangle();

        const ModelData model = ParseModel(file, ModelPath);

        EXPECT_EQ(model.clips[0].tracks[1].translation.interpolation, Interpolation::Step);
    }

    TEST(ModelHelpersTests, ReadsTheMaterialBaseColor) {
        const auto file = SkinnedTriangle();

        const ModelData model = ParseModel(file, ModelPath);

        EXPECT_TRUE(Vector4f::NearlyEqual(model.materials[0].baseColor, Vector4f{1.0f, 0.5f, 0.25f, 1.0f}));
    }

    TEST(ModelHelpersTests, StaticMeshesAreMovedByTheirNode) {
        const auto file = MovedStaticTriangle();

        const ModelData model = ParseModel(file, ModelPath);

        EXPECT_TRUE(Vector3f::NearlyEqual(model.meshes[0].vertices[0].position, Vector3f{5.0f, 0.0f, 0.0f}));
    }

    TEST(ModelHelpersTests, MeshesWithoutIndicesAreDrawnInVertexOrder) {
        const auto file = MovedStaticTriangle();

        const ModelData model = ParseModel(file, ModelPath);

        EXPECT_EQ(model.meshes[0].indices, (std::vector<std::uint32_t>{0, 1, 2}));
    }

    TEST(ModelHelpersTests, PrimitivesWithoutAMaterialUseTheDefaultOne) {
        const auto file = MovedStaticTriangle();

        const ModelData model = ParseModel(file, ModelPath);

        EXPECT_EQ(model.meshes[0].material, model.materials.size() - 1);
    }

    TEST(ModelHelpersTests, AnExternalTextureKeepsItsDecodedPathRelativeToTheModel) {
        const auto file = TexturedTriangle("skin%20uv.png");

        const ModelData model = ParseModel(file, ModelPath);

        EXPECT_EQ(model.materials[0].texturePath, std::filesystem::path("skin uv.png"));
    }

    TEST(ModelHelpersTests, AnEmbeddedTextureIsDecoded) {
        const auto file = TexturedTriangle("data:image/png;base64,YWJj");

        const ModelData model = ParseModel(file, ModelPath);

        EXPECT_EQ(model.materials[0].embeddedTexture, (std::vector<std::uint8_t>{'a', 'b', 'c'}));
    }

    TEST(ModelHelpersTests, DataThatIsNotGLTFIsRejected) {
        const std::string text = "not a model";
        const std::vector<std::uint8_t> file(text.begin(), text.end());

        EXPECT_THROW(static_cast<void>(ParseModel(file, ModelPath)), std::runtime_error);
    }

    TEST(ModelHelpersTests, AnEmptyFileIsRejected) {
        const std::vector<std::uint8_t> file;

        EXPECT_THROW(static_cast<void>(ParseModel(file, ModelPath)), std::runtime_error);
    }

    TEST(ModelHelpersTests, SkinsWithMoreJointsThanTheShaderSupportsAreRejected) {
        GLTFBuilder builder;
        Json nodes = Json::array();
        Json joints = Json::array();
        for (int index = 0; index < 65; ++index) {
            nodes.push_back(Json::object());
            joints.push_back(index);
        }
        builder.gltf["nodes"] = nodes;
        builder.gltf["skins"] = Json::array({{{"joints", joints}}});
        const auto file = builder.Build();

        EXPECT_THROW(static_cast<void>(ParseModel(file, ModelPath)), std::runtime_error);
    }

    TEST(ModelHelpersTests, AMeshOnAnotherSkinWithTheSameJointsIsSkinned) {
        const auto file = CharacterWithAHat();

        const ModelData model = ParseModel(file, ModelPath);

        EXPECT_TRUE(model.meshes[1].skinned);
    }

    TEST(ModelHelpersTests, AStaticMeshUnderAJointIsBoundToThatJoint) {
        const auto file = CharacterWithAHat();

        const ModelData model = ParseModel(file, ModelPath);

        EXPECT_TRUE(model.meshes[0].skinned);
        EXPECT_EQ(model.meshes[0].vertices[0].joints, (std::array<std::uint8_t, 4>{1, 0, 0, 0}));
        EXPECT_TRUE(Vector4f::NearlyEqual(model.meshes[0].vertices[0].weights, Vector4f{1.0f, 0.0f, 0.0f, 0.0f}));
    }

    TEST(ModelHelpersTests, AStaticMeshUnderAJointKeepsItsOffsetFromTheJoint) {
        const auto file = CharacterWithAHat();

        const ModelData model = ParseModel(file, ModelPath);

        EXPECT_TRUE(Vector3f::NearlyEqual(model.meshes[0].vertices[1].position, Vector3f{1.0f, 1.0f, 0.0f}));
    }
}
