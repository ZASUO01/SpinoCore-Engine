// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <memory>
#include <utility>
#include <gtest/gtest.h>
#include "Core/Actors/EngineActorsManager.h"
#include "Physics/CollisionProcessor.h"
#include "SpinoCore/Core/Actors/Actor.h"
#include "SpinoCore/Core/Actors/Components/BoxColliderComponent.h"
#include "SpinoCore/Core/Actors/Components/ColliderComponent.h"
#include "SpinoCore/Core/Actors/Components/RigidBodyComponent.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Core::Actors::Actor;
    using SpinoCore::Core::Actors::ActorInput;
    using SpinoCore::Core::Actors::EngineActorsManager;
    using SpinoCore::Core::Actors::Components::BoxColliderComponent;
    using SpinoCore::Core::Actors::Components::BoxColliderInput;
    using SpinoCore::Core::Actors::Components::ColliderComponent;
    using SpinoCore::Core::Actors::Components::RigidBodyComponent;
    using SpinoCore::Core::Actors::Components::RigidBodyInput;
    using SpinoCore::Math::Vector3f;
    using SpinoCore::Physics::CollisionProcessor;

    class StillActor final : public Actor {
    public:
        explicit StillActor(const ActorInput& input) : Actor(input) {}

    protected:
        void OnUpdate(float) override {}
        void OnProcessInput(const SpinoCore::Core::Input::InputState&) override {}
    };

    [[nodiscard]] BoxColliderInput UnitBox(const bool isStatic, const bool isTrigger = false) {
        BoxColliderInput input{};
        input.extent = Vector3f{0.5f, 0.5f, 0.5f};
        input.isStatic = isStatic;
        input.isTrigger = isTrigger;
        input.minRestitutionVelocity = 0.0f;
        return input;
    }

    class CollisionProcessorTests : public testing::Test {
    protected:
        Actor& AddBox(const Vector3f& position, const BoxColliderInput& collider, const Vector3f& scale = Vector3f::One()) {
            auto actor = std::make_unique<StillActor>(ActorInput{.position = position, .scale = scale});
            actor->AddComponent(std::make_unique<BoxColliderComponent>(*actor, collider));
            Actor& added = *actor;
            actors.AddActor(std::move(actor));
            return added;
        }

        static RigidBodyComponent& AddBody(Actor& actor, const Vector3f velocity) {
            RigidBodyInput input{};
            input.applyGravity = false;
            auto body = std::make_unique<RigidBodyComponent>(actor, input);
            body->SetVelocity(velocity);
            RigidBodyComponent& added = *body;
            actor.AddComponent(std::move(body));
            return added;
        }

        static ColliderComponent& ColliderOf(const Actor& actor) {
            return *actor.GetComponent<ColliderComponent>();
        }

        CollisionProcessor processor{};
        EngineActorsManager actors{&processor};
    };

    TEST_F(CollisionProcessorTests, BoxesApartDoNotFireTheCallback) {
        int calls = 0;
        Actor& moving = AddBox(Vector3f::Zero(), UnitBox(false));
        AddBox(Vector3f{3.0f, 0.0f, 0.0f}, UnitBox(true));
        ColliderOf(moving).SetCollisionCallBack([&calls](ColliderComponent&) { ++calls; });

        processor.Update();

        EXPECT_EQ(calls, 0);
    }

    TEST_F(CollisionProcessorTests, TouchingBoxesFireTheCallbackWithTheOtherCollider) {
        const ColliderComponent* touched = nullptr;
        Actor& moving = AddBox(Vector3f{0.8f, 0.0f, 0.0f}, UnitBox(false));
        Actor& wall = AddBox(Vector3f::Zero(), UnitBox(true));
        ColliderOf(moving).SetCollisionCallBack([&touched](ColliderComponent& other) { touched = &other; });

        processor.Update();

        EXPECT_EQ(touched, &ColliderOf(wall));
    }

    TEST_F(CollisionProcessorTests, TouchingBoxesAreMarkedAsColliding) {
        Actor& moving = AddBox(Vector3f{0.8f, 0.0f, 0.0f}, UnitBox(false));
        AddBox(Vector3f::Zero(), UnitBox(true));

        processor.Update();

        EXPECT_TRUE(ColliderOf(moving).IsColliding());
    }

    TEST_F(CollisionProcessorTests, ATriggerFiresTheCallback) {
        int calls = 0;
        AddBox(Vector3f{0.8f, 0.0f, 0.0f}, UnitBox(false));
        Actor& zone = AddBox(Vector3f::Zero(), UnitBox(true, true));
        ColliderOf(zone).SetCollisionCallBack([&calls](ColliderComponent&) { ++calls; });

        processor.Update();

        EXPECT_EQ(calls, 1);
    }

    TEST_F(CollisionProcessorTests, ATriggerDoesNotPushAnything) {
        Actor& moving = AddBox(Vector3f{0.8f, 0.0f, 0.0f}, UnitBox(false));
        AddBox(Vector3f::Zero(), UnitBox(true, true));

        processor.Update();

        EXPECT_TRUE(Vector3f::NearlyEqual(moving.GetPosition(), Vector3f{0.8f, 0.0f, 0.0f}));
    }

    TEST_F(CollisionProcessorTests, ADynamicBoxIsPushedOutOfAStaticOne) {
        Actor& moving = AddBox(Vector3f{0.8f, 0.0f, 0.0f}, UnitBox(false));
        AddBox(Vector3f::Zero(), UnitBox(true));

        processor.Update();

        EXPECT_TRUE(Vector3f::NearlyEqual(moving.GetPosition(), Vector3f{1.0f, 0.0f, 0.0f}));
    }

    TEST_F(CollisionProcessorTests, AStaticBoxIsNeverPushed) {
        AddBox(Vector3f{0.8f, 0.0f, 0.0f}, UnitBox(false));
        Actor& wall = AddBox(Vector3f::Zero(), UnitBox(true));

        processor.Update();

        EXPECT_TRUE(Vector3f::NearlyEqual(wall.GetPosition(), Vector3f::Zero()));
    }

    TEST_F(CollisionProcessorTests, TwoDynamicBoxesShareTheSeparation) {
        Actor& left = AddBox(Vector3f::Zero(), UnitBox(false));
        AddBox(Vector3f{0.8f, 0.0f, 0.0f}, UnitBox(false));

        processor.Update();

        EXPECT_TRUE(Vector3f::NearlyEqual(left.GetPosition(), Vector3f{-0.1f, 0.0f, 0.0f}));
    }

    TEST_F(CollisionProcessorTests, StaticBoxesAreNotTestedAgainstEachOther) {
        int calls = 0;
        Actor& first = AddBox(Vector3f::Zero(), UnitBox(true));
        AddBox(Vector3f{0.5f, 0.0f, 0.0f}, UnitBox(true));
        ColliderOf(first).SetCollisionCallBack([&calls](ColliderComponent&) { ++calls; });

        processor.Update();

        EXPECT_EQ(calls, 0);
    }

    TEST_F(CollisionProcessorTests, TheActorScaleEnlargesItsBox) {
        int calls = 0;
        Actor& moving = AddBox(Vector3f{1.2f, 0.0f, 0.0f}, UnitBox(false), Vector3f{2.0f, 2.0f, 2.0f});
        AddBox(Vector3f::Zero(), UnitBox(true));
        ColliderOf(moving).SetCollisionCallBack([&calls](ColliderComponent&) { ++calls; });

        processor.Update();

        EXPECT_EQ(calls, 1);
    }

    TEST_F(CollisionProcessorTests, ADisabledColliderIsIgnored) {
        int calls = 0;
        Actor& moving = AddBox(Vector3f{0.8f, 0.0f, 0.0f}, UnitBox(false));
        AddBox(Vector3f::Zero(), UnitBox(true));
        ColliderOf(moving).SetCollisionCallBack([&calls](ColliderComponent&) { ++calls; });
        ColliderOf(moving).SetEnabled(false);

        processor.Update();

        EXPECT_EQ(calls, 0);
    }

    TEST_F(CollisionProcessorTests, FullRestitutionBouncesTheBodyBack) {
        BoxColliderInput bouncy = UnitBox(false);
        bouncy.restitutionCoefficient = 1.0f;
        BoxColliderInput bouncyWall = UnitBox(true);
        bouncyWall.restitutionCoefficient = 1.0f;
        Actor& ball = AddBox(Vector3f{0.8f, 0.0f, 0.0f}, bouncy);
        AddBox(Vector3f::Zero(), bouncyWall);
        const RigidBodyComponent& body = AddBody(ball, Vector3f{-5.0f, 0.0f, 0.0f});

        processor.Update();

        EXPECT_TRUE(Vector3f::NearlyEqual(body.GetVelocity(), Vector3f{5.0f, 0.0f, 0.0f}));
    }

    TEST_F(CollisionProcessorTests, ZeroRestitutionStopsTheBodyAlongTheNormal) {
        Actor& ball = AddBox(Vector3f{0.8f, 0.0f, 0.0f}, UnitBox(false));
        AddBox(Vector3f::Zero(), UnitBox(true));
        const RigidBodyComponent& body = AddBody(ball, Vector3f{-5.0f, 0.0f, 0.0f});

        processor.Update();

        EXPECT_TRUE(Vector3f::NearlyEqual(body.GetVelocity(), Vector3f::Zero()));
    }

    TEST_F(CollisionProcessorTests, BelowTheMinimumSpeedThereIsNoBounce) {
        BoxColliderInput bouncy = UnitBox(false);
        bouncy.restitutionCoefficient = 1.0f;
        bouncy.minRestitutionVelocity = 10.0f;
        BoxColliderInput bouncyWall = UnitBox(true);
        bouncyWall.restitutionCoefficient = 1.0f;
        bouncyWall.minRestitutionVelocity = 10.0f;
        Actor& ball = AddBox(Vector3f{0.8f, 0.0f, 0.0f}, bouncy);
        AddBox(Vector3f::Zero(), bouncyWall);
        const RigidBodyComponent& body = AddBody(ball, Vector3f{-5.0f, 0.0f, 0.0f});

        processor.Update();

        EXPECT_TRUE(Vector3f::NearlyEqual(body.GetVelocity(), Vector3f::Zero()));
    }

    TEST_F(CollisionProcessorTests, ABodyMovingAwayKeepsItsVelocity) {
        Actor& ball = AddBox(Vector3f{0.8f, 0.0f, 0.0f}, UnitBox(false));
        AddBox(Vector3f::Zero(), UnitBox(true));
        const RigidBodyComponent& body = AddBody(ball, Vector3f{2.0f, 0.0f, 0.0f});

        processor.Update();

        EXPECT_TRUE(Vector3f::NearlyEqual(body.GetVelocity(), Vector3f{2.0f, 0.0f, 0.0f}));
    }
}
