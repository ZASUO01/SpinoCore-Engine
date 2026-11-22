// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <gtest/gtest.h>
#include "../../Graphics/GLTestContext.h"
#include "Assets/AssetCache.h"
#include "Core/Rendering/Drawer/DrawCommands.h"
#include "Graphics/Resources/Model.h"
#include "Graphics/Resources/ModelData.h"
#include "SpinoCore/Core/Actors/Actor.h"
#include "SpinoCore/Core/Actors/Components/AnimatorComponent.h"
#include "SpinoCore/Core/Actors/Components/ModelComponent.h"
#include "SpinoCore/Math/MatrixFactory.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Assets::AssetCache;
    using SpinoCore::Core::Actors::Actor;
    using SpinoCore::Core::Actors::ActorInput;
    using SpinoCore::Core::Actors::Components::AnimatorComponent;
    using SpinoCore::Core::Actors::Components::ModelComponent;
    using SpinoCore::Core::Rendering::ModelDrawList;
    using SpinoCore::Graphics::Resources::AnimationClip;
    using SpinoCore::Graphics::Resources::Bone;
    using SpinoCore::Graphics::Resources::BoneTrack;
    using SpinoCore::Graphics::Resources::Keyframes;
    using SpinoCore::Graphics::Resources::MeshData;
    using SpinoCore::Graphics::Resources::Model;
    using SpinoCore::Graphics::Resources::ModelData;
    using SpinoCore::Graphics::Resources::ModelVertex;
    using SpinoCore::Math::Vector3f;
    using SpinoCore::Tests::GLTestContext;
    namespace MatrixFactory = SpinoCore::Math::MatrixFactory;

    class StillActor final : public Actor {
    public:
        explicit StillActor(const ActorInput& input = {}) : Actor(input) {}

    protected:
        void OnUpdate(float) override {}
        void OnProcessInput(const SpinoCore::Core::Input::InputState&) override {}
    };

    [[nodiscard]] ModelData SlidingTriangle() {
        ModelData data{};
        MeshData mesh{};
        mesh.vertices = {ModelVertex{}, ModelVertex{}, ModelVertex{}};
        mesh.indices = {0, 1, 2};
        mesh.skinned = true;
        data.meshes.push_back(mesh);

        data.skeleton.bones.push_back(Bone{.name = "root"});
        data.skeleton.evaluationOrder = {0};

        AnimationClip slide{.name = "slide", .duration = 1.0f, .tracks = {}};
        slide.tracks.push_back(BoneTrack{
            .bone = 0,
            .translation = Keyframes<Vector3f>{.times = {0.0f, 1.0f}, .values = {Vector3f::Zero(), Vector3f{2.0f, 0.0f, 0.0f}}},
            .rotation = {},
            .scale = {},
        });
        data.clips.push_back(slide);
        data.clips.push_back(AnimationClip{.name = "idle", .duration = 1.0f, .tracks = {}});
        return data;
    }

    class ModelComponentsTests : public GLTestContext {
    protected:
        void SetUp() override {
            GLTestContext::SetUp();
            if (HasFatalFailure()) return;
            models.emplace(std::make_unique<const Model>());
            models->Add("hero", std::make_unique<const Model>(SlidingTriangle(), std::vector{Model::Material{}}));
            drawList.models = &*models;
            actor.emplace(ActorInput{.position = Vector3f{0.0f, 0.0f, -3.0f}});
        }

        void TearDown() override {
            actor.reset();
            models.reset();
            GLTestContext::TearDown();
        }

        ModelComponent& AddModel(std::string id) {
            auto component = std::make_unique<ModelComponent>(*actor, std::move(id));
            ModelComponent& added = *component;
            actor->AddComponent(std::move(component));
            return added;
        }

        AnimatorComponent& AddAnimator() {
            auto component = std::make_unique<AnimatorComponent>(*actor);
            AnimatorComponent& added = *component;
            actor->AddComponent(std::move(component));
            return added;
        }

        void Render() {
            drawList.commands.clear();
            actor->GetDrawCommands(drawList);
        }

        std::optional<AssetCache<Model>> models{};
        ModelDrawList drawList{};
        std::optional<StillActor> actor{};
    };

    TEST_F(ModelComponentsTests, TheModelComponentDrawsTheModelCachedUnderItsId) {
        AddModel("hero");

        Render();

        ASSERT_EQ(drawList.commands.size(), 1u);
        EXPECT_EQ(drawList.commands[0].model, &models->Get("hero"));
    }

    TEST_F(ModelComponentsTests, AnUnknownModelIdFallsBackToTheDefaultModel) {
        AddModel("ghost");

        Render();

        EXPECT_EQ(drawList.commands[0].model, &models->GetDefault());
    }

    TEST_F(ModelComponentsTests, TheModelIsDrawnWhereTheActorIs) {
        AddModel("hero");

        Render();

        EXPECT_TRUE(Vector3f::NearlyEqual(MatrixFactory::GetTranslation(drawList.commands[0].world), Vector3f{0.0f, 0.0f, -3.0f}));
    }

    TEST_F(ModelComponentsTests, WithoutAnAnimatorTheModelIsDrawnWithoutBones) {
        AddModel("hero");

        Render();

        EXPECT_TRUE(drawList.commands[0].bones.empty());
    }

    TEST_F(ModelComponentsTests, WithAnAnimatorTheModelIsDrawnWithItsBones) {
        AddModel("hero");
        AnimatorComponent& animator = AddAnimator();
        Render();
        animator.Play("slide");
        actor->Update(0.5f);

        Render();

        EXPECT_EQ(drawList.commands[0].bones.size(), 1u);
    }

    TEST_F(ModelComponentsTests, PlayingAdvancesTheClipTime) {
        AddModel("hero");
        AnimatorComponent& animator = AddAnimator();
        Render();
        animator.Play("slide");

        actor->Update(0.25f);

        EXPECT_FLOAT_EQ(animator.GetTime(), 0.25f);
    }

    TEST_F(ModelComponentsTests, ThePoseIsInterpolatedBetweenKeyframes) {
        AddModel("hero");
        AnimatorComponent& animator = AddAnimator();
        Render();
        animator.Play("slide");

        actor->Update(0.5f);

        EXPECT_TRUE(Vector3f::NearlyEqual(MatrixFactory::GetTranslation(animator.GetBoneMatrices()[0]), Vector3f{1.0f, 0.0f, 0.0f}));
    }

    TEST_F(ModelComponentsTests, ALoopingClipWrapsAroundItsEnd) {
        AddModel("hero");
        AnimatorComponent& animator = AddAnimator();
        Render();
        animator.Play("slide", true);

        actor->Update(1.25f);

        EXPECT_FLOAT_EQ(animator.GetTime(), 0.25f);
    }

    TEST_F(ModelComponentsTests, AClipThatDoesNotLoopStopsAtItsEnd) {
        AddModel("hero");
        AnimatorComponent& animator = AddAnimator();
        Render();
        animator.Play("slide", false);

        actor->Update(1.5f);

        EXPECT_FALSE(animator.IsPlaying());
    }

    TEST_F(ModelComponentsTests, AClipThatDoesNotLoopHoldsItsLastPose) {
        AddModel("hero");
        AnimatorComponent& animator = AddAnimator();
        Render();
        animator.Play("slide", false);

        actor->Update(1.5f);

        EXPECT_TRUE(Vector3f::NearlyEqual(MatrixFactory::GetTranslation(animator.GetBoneMatrices()[0]), Vector3f{2.0f, 0.0f, 0.0f}));
    }

    TEST_F(ModelComponentsTests, PlayingAnotherClipStartsItFromTheBeginning) {
        AddModel("hero");
        AnimatorComponent& animator = AddAnimator();
        Render();
        animator.Play("slide");
        actor->Update(0.5f);

        animator.Play("idle");

        EXPECT_EQ(animator.GetCurrentClip(), "idle");
        EXPECT_FLOAT_EQ(animator.GetTime(), 0.0f);
    }

    TEST_F(ModelComponentsTests, StoppingFreezesTheClipTime) {
        AddModel("hero");
        AnimatorComponent& animator = AddAnimator();
        Render();
        animator.Play("slide");
        actor->Update(0.25f);

        animator.Stop();
        actor->Update(0.5f);

        EXPECT_FLOAT_EQ(animator.GetTime(), 0.25f);
    }

    TEST_F(ModelComponentsTests, AnUnknownClipStopsTheAnimator) {
        AddModel("hero");
        AnimatorComponent& animator = AddAnimator();
        Render();
        animator.Play("fly");

        actor->Update(0.1f);

        EXPECT_FALSE(animator.IsPlaying());
    }

    TEST_F(ModelComponentsTests, WithoutAClipTheAnimatorHoldsTheBindPose) {
        AddModel("hero");
        AnimatorComponent& animator = AddAnimator();
        Render();

        actor->Update(0.1f);

        EXPECT_TRUE(Vector3f::NearlyEqual(MatrixFactory::GetTranslation(animator.GetBoneMatrices()[0]), Vector3f::Zero()));
    }

    TEST_F(ModelComponentsTests, TheAnimatorWaitsForTheModelToBeResolved) {
        AddModel("hero");
        AnimatorComponent& animator = AddAnimator();
        animator.Play("slide");

        actor->Update(0.5f);

        EXPECT_TRUE(animator.GetBoneMatrices().empty());
    }
}
