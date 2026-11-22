// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <memory>
#include <utility>
#include <gtest/gtest.h>
#include "Core/Actors/EngineActorsManager.h"
#include "Physics/CollisionProcessor.h"
#include "SpinoCore/Core/Actors/Actor.h"
#include "SpinoCore/Core/Actors/Components/BoxColliderComponent.h"
#include "SpinoCore/Core/Actors/Components/RigidBodyComponent.h"

namespace {
    using SpinoCore::Core::Actors::Actor;
    using SpinoCore::Core::Actors::ActorInput;
    using SpinoCore::Core::Actors::EngineActorsManager;
    using SpinoCore::Core::Actors::Components::BoxColliderComponent;
    using SpinoCore::Core::Actors::Components::BoxColliderInput;
    using SpinoCore::Core::Actors::Components::RigidBodyComponent;
    using SpinoCore::Physics::CollisionProcessor;

    class StillActor final : public Actor {
    public:
        StillActor() : Actor(ActorInput{}) {}

    protected:
        void OnUpdate(float) override {}
        void OnProcessInput(const SpinoCore::Core::Input::InputState&) override {}
    };

    class ColliderSpawningActor final : public Actor {
    public:
        ColliderSpawningActor() : Actor(ActorInput{}) {}

    protected:
        void OnUpdate(float) override {
            if (GetComponents().empty()) AddComponent(std::make_unique<BoxColliderComponent>(*this, BoxColliderInput{}));
        }

        void OnProcessInput(const SpinoCore::Core::Input::InputState&) override {}
    };

    class ColliderRegistrationTests : public testing::Test {
    protected:
        Actor& AddActorWithCollider() {
            auto actor = std::make_unique<StillActor>();
            actor->AddComponent(std::make_unique<BoxColliderComponent>(*actor, BoxColliderInput{}));
            Actor& added = *actor;
            actors.AddActor(std::move(actor));
            return added;
        }

        CollisionProcessor processor{};
        EngineActorsManager actors{&processor};
    };

    TEST_F(ColliderRegistrationTests, ACollidingActorAddedToTheSceneIsRegistered) {
        AddActorWithCollider();

        EXPECT_EQ(processor.GetColliderCount(), 1u);
    }

    TEST_F(ColliderRegistrationTests, AColliderAddedToAnActorInTheSceneIsRegistered) {
        Actor& actor = *actors.AddActor(std::make_unique<StillActor>());

        actor.AddComponent(std::make_unique<BoxColliderComponent>(actor, BoxColliderInput{}));

        EXPECT_EQ(processor.GetColliderCount(), 1u);
    }

    TEST_F(ColliderRegistrationTests, OtherComponentsAreNotRegistered) {
        Actor& actor = *actors.AddActor(std::make_unique<StillActor>());

        actor.AddComponent(std::make_unique<RigidBodyComponent>(actor));

        EXPECT_EQ(processor.GetColliderCount(), 0u);
    }

    TEST_F(ColliderRegistrationTests, AColliderAddedDuringAnUpdateIsRegisteredAfterIt) {
        actors.AddActor(std::make_unique<ColliderSpawningActor>());

        actors.UpdateActors(0.016f);

        EXPECT_EQ(processor.GetColliderCount(), 1u);
    }

    TEST_F(ColliderRegistrationTests, RemovingTheColliderUnregistersIt) {
        Actor& actor = AddActorWithCollider();

        actor.RemoveComponent(actor.GetComponents().front().get());

        EXPECT_EQ(processor.GetColliderCount(), 0u);
    }

    TEST_F(ColliderRegistrationTests, DestroyingTheActorUnregistersItsCollider) {
        Actor& actor = AddActorWithCollider();
        actor.SetState(Actor::State::Destroy);

        actors.UpdateActors(0.016f);

        EXPECT_EQ(processor.GetColliderCount(), 0u);
    }

    TEST_F(ColliderRegistrationTests, CleaningTheSceneUnregistersEveryCollider) {
        AddActorWithCollider();
        AddActorWithCollider();

        actors.CleanUp();

        EXPECT_EQ(processor.GetColliderCount(), 0u);
    }

    TEST_F(ColliderRegistrationTests, AnActorOutsideTheSceneRegistersNothing) {
        StillActor actor;

        actor.AddComponent(std::make_unique<BoxColliderComponent>(actor, BoxColliderInput{}));

        EXPECT_EQ(processor.GetColliderCount(), 0u);
    }
}
