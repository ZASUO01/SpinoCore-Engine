// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <memory>
#include <utility>
#include <gtest/gtest.h>
#include "Core/Actors/EngineActorsManager.h"
#include "Physics/ColliderShapes.h"
#include "Physics/CollisionProcessor.h"
#include "Physics/Shapes.h"
#include "SpinoCore/Core/Actors/Actor.h"
#include "SpinoCore/Core/Actors/Components/BoxColliderComponent.h"
#include "SpinoCore/Core/Actors/Components/CapsuleColliderComponent.h"
#include "SpinoCore/Core/Actors/Components/ColliderComponent.h"
#include "SpinoCore/Core/Actors/Components/PlaneColliderComponent.h"
#include "SpinoCore/Core/Actors/Components/SphereColliderComponent.h"
#include "SpinoCore/Math/Math.h"
#include "SpinoCore/Math/Quaternion.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Core::Actors::Actor;
    using SpinoCore::Core::Actors::ActorInput;
    using SpinoCore::Core::Actors::EngineActorsManager;
    using SpinoCore::Core::Actors::Components::BoxColliderComponent;
    using SpinoCore::Core::Actors::Components::BoxColliderInput;
    using SpinoCore::Core::Actors::Components::CapsuleColliderComponent;
    using SpinoCore::Core::Actors::Components::CapsuleColliderInput;
    using SpinoCore::Core::Actors::Components::ColliderComponent;
    using SpinoCore::Core::Actors::Components::ColliderType;
    using SpinoCore::Core::Actors::Components::PlaneColliderComponent;
    using SpinoCore::Core::Actors::Components::PlaneColliderInput;
    using SpinoCore::Core::Actors::Components::SphereColliderComponent;
    using SpinoCore::Core::Actors::Components::SphereColliderInput;
    using SpinoCore::Math::Quaternionf;
    using SpinoCore::Math::Vector3f;
    using SpinoCore::Physics::CollisionProcessor;

    class StillActor final : public Actor {
    public:
        explicit StillActor(const ActorInput& input) : Actor(input) {}

    protected:
        void OnUpdate(float) override {}
        void OnProcessInput(const SpinoCore::Core::Input::InputState&) override {}
    };

    [[nodiscard]] SphereColliderInput DynamicSphere() {
        SphereColliderInput input{};
        input.radius = 0.5f;
        input.isStatic = false;
        return input;
    }

    [[nodiscard]] BoxColliderInput DynamicBox() {
        BoxColliderInput input{};
        input.extent = Vector3f{0.5f, 0.5f, 0.5f};
        input.isStatic = false;
        return input;
    }

    class ColliderPairsTests : public testing::Test {
    protected:
        template <typename Collider, typename Input>
        Actor& Add(const Vector3f& position, const Input& input, const ActorInput& actorInput = {}) {
            ActorInput placed = actorInput;
            placed.position = position;
            auto actor = std::make_unique<StillActor>(placed);
            actor->AddComponent(std::make_unique<Collider>(*actor, input));
            Actor& added = *actor;
            actors.AddActor(std::move(actor));
            return added;
        }

        CollisionProcessor processor{};
        EngineActorsManager actors{&processor};
    };

    TEST_F(ColliderPairsTests, ASphereRegisteredBeforeABoxIsPushedAwayFromIt) {
        Actor& sphere = Add<SphereColliderComponent>(Vector3f{0.8f, 0.0f, 0.0f}, DynamicSphere());
        Add<BoxColliderComponent>(Vector3f::Zero(), DynamicBox());

        processor.Update();

        EXPECT_GT(sphere.GetPosition().x, 0.8f);
    }

    TEST_F(ColliderPairsTests, ASphereRegisteredAfterABoxIsPushedAwayFromIt) {
        Add<BoxColliderComponent>(Vector3f::Zero(), DynamicBox());
        Actor& sphere = Add<SphereColliderComponent>(Vector3f{0.8f, 0.0f, 0.0f}, DynamicSphere());

        processor.Update();

        EXPECT_GT(sphere.GetPosition().x, 0.8f);
    }

    TEST_F(ColliderPairsTests, ACapsuleAndASphereSeparateWhicheverComesFirst) {
        CapsuleColliderInput capsule{};
        capsule.isStatic = false;
        Actor& capsuleActor = Add<CapsuleColliderComponent>(Vector3f::Zero(), capsule);
        Add<SphereColliderComponent>(Vector3f{0.8f, 0.0f, 0.0f}, DynamicSphere());

        processor.Update();

        EXPECT_LT(capsuleActor.GetPosition().x, 0.0f);
    }

    TEST_F(ColliderPairsTests, ASphereFallingIntoTheGroundIsPushedUp) {
        Add<PlaneColliderComponent>(Vector3f::Zero(), PlaneColliderInput{});
        Actor& sphere = Add<SphereColliderComponent>(Vector3f{0.0f, 0.3f, 0.0f}, DynamicSphere());

        processor.Update();

        EXPECT_NEAR(sphere.GetPosition().y, 0.5f, 1e-4f);
    }

    TEST_F(ColliderPairsTests, ThePlaneNeverMoves) {
        Actor& ground = Add<PlaneColliderComponent>(Vector3f::Zero(), PlaneColliderInput{});
        Add<SphereColliderComponent>(Vector3f{0.0f, 0.3f, 0.0f}, DynamicSphere());

        processor.Update();

        EXPECT_TRUE(Vector3f::NearlyEqual(ground.GetPosition(), Vector3f::Zero()));
    }

    TEST_F(ColliderPairsTests, APlaneAskedToBeDynamicIsStatic) {
        PlaneColliderInput input{};
        input.isStatic = false;
        Actor& ground = Add<PlaneColliderComponent>(Vector3f::Zero(), input);

        EXPECT_TRUE(ground.GetComponent<ColliderComponent>()->IsStatic());
    }

    TEST_F(ColliderPairsTests, EachColliderKeepsItsOwnTypeWhateverTheInputSays) {
        SphereColliderInput input = DynamicSphere();
        input.type = ColliderType::Box;
        Actor& sphere = Add<SphereColliderComponent>(Vector3f::Zero(), input);

        EXPECT_EQ(sphere.GetComponent<ColliderComponent>()->GetType(), ColliderType::Sphere);
    }

    TEST_F(ColliderPairsTests, TheActorScaleEnlargesTheSphereRadius) {
        Actor& sphere = Add<SphereColliderComponent>(Vector3f::Zero(), DynamicSphere(), ActorInput{.scale = Vector3f{1.0f, 3.0f, 2.0f}});

        const auto shape = SpinoCore::Physics::SphereOf(*sphere.GetComponent<SphereColliderComponent>());

        EXPECT_NEAR(shape.radius, 1.5f, 1e-5f);
    }

    TEST_F(ColliderPairsTests, ARotatedCapsuleLiesAlongTheRotatedAxis) {
        CapsuleColliderInput input{};
        input.halfHeight = 1.0f;
        const ActorInput rotated{.rotation = Quaternionf{Vector3f::UnitZ(), SpinoCore::Math::Constants::PIOver2<float>}};
        Actor& capsule = Add<CapsuleColliderComponent>(Vector3f::Zero(), input, rotated);

        const auto shape = SpinoCore::Physics::CapsuleOf(*capsule.GetComponent<CapsuleColliderComponent>());

        EXPECT_TRUE(Vector3f::NearlyEqual(shape.end, Vector3f{-1.0f, 0.0f, 0.0f}));
    }

    TEST_F(ColliderPairsTests, ATiltedPlaneHasATiltedNormal) {
        const ActorInput tilted{.rotation = Quaternionf{Vector3f::UnitZ(), SpinoCore::Math::Constants::PIOver2<float>}};
        Actor& wall = Add<PlaneColliderComponent>(Vector3f::Zero(), PlaneColliderInput{}, tilted);

        const auto shape = SpinoCore::Physics::PlaneOf(*wall.GetComponent<PlaneColliderComponent>());

        EXPECT_TRUE(Vector3f::NearlyEqual(shape.normal, Vector3f::NegUnitX()));
    }
}
