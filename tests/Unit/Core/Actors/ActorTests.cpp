// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <memory>
#include <gtest/gtest.h>
#include "SpinoCore/Core/Actors/Actor.h"
#include "SpinoCore/Core/Actors/Components/Component.h"
#include "SpinoCore/Math/Math.h"
#include "SpinoCore/Math/Quaternion.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Core::Actors::Actor;
    using SpinoCore::Core::Actors::ActorInput;
    using SpinoCore::Math::Quaternionf;
    using SpinoCore::Math::Vector3f;

    constexpr float QuarterTurn = SpinoCore::Math::Constants::PIOver2<float>;

    class StillActor final : public Actor {
    public:
        explicit StillActor(const ActorInput& input = {}) : Actor(input) {}

    protected:
        void OnUpdate(float) override {}
        void OnProcessInput(const SpinoCore::Core::Input::InputState&) override {}
    };

    class CountingComponent final : public SpinoCore::Core::Actors::Components::Component {
    public:
        CountingComponent(Actor& actor, int& updates) : Component(actor), mUpdates(updates) {}

        void Update(float) override { ++mUpdates; }
        void GetDrawCommand(SpinoCore::Core::Rendering::ModelDrawList&) override {}

    private:
        int& mUpdates;
    };

    class SpawningActor final : public Actor {
    public:
        explicit SpawningActor(int& spawnedUpdates) : mSpawnedUpdates(spawnedUpdates) {}

    protected:
        void OnUpdate(float) override {
            if (GetComponents().empty()) AddComponent(std::make_unique<CountingComponent>(*this, mSpawnedUpdates));
        }

        void OnProcessInput(const SpinoCore::Core::Input::InputState&) override {}

    private:
        int& mSpawnedUpdates;
    };

    [[nodiscard]] Vector3f WorldPointOf(Actor& actor, const Vector3f& localPoint) {
        return actor.GetWorldTransform().TransformPoint(localPoint);
    }

    TEST(ActorTests, ANewActorSitsAtTheOriginWithoutRotationOrScale) {
        StillActor actor;

        const Vector3f point = WorldPointOf(actor, Vector3f{1.0f, 2.0f, 3.0f});

        EXPECT_TRUE(Vector3f::NearlyEqual(point, Vector3f{1.0f, 2.0f, 3.0f}));
    }

    TEST(ActorTests, TheWorldTransformScalesThenRotatesThenTranslates) {
        StillActor actor{ActorInput{
            .position = Vector3f{1.0f, 2.0f, 3.0f},
            .scale = Vector3f{2.0f, 3.0f, 4.0f},
            .rotation = Quaternionf{Vector3f::UnitY(), QuarterTurn}
        }};

        const Vector3f point = WorldPointOf(actor, Vector3f{1.0f, 0.0f, 0.0f});

        EXPECT_TRUE(Vector3f::NearlyEqual(point, Vector3f{1.0f, 2.0f, 1.0f}));
    }

    TEST(ActorTests, AQuaternionAroundZRotatesXOntoY) {
        StillActor actor{ActorInput{.rotation = Quaternionf{Vector3f::UnitZ(), QuarterTurn}}};

        const Vector3f point = WorldPointOf(actor, Vector3f{1.0f, 0.0f, 0.0f});

        EXPECT_TRUE(Vector3f::NearlyEqual(point, Vector3f{0.0f, 1.0f, 0.0f}));
    }

    TEST(ActorTests, TheRotationIsKeptNormalized) {
        StillActor actor;

        actor.SetRotation(Quaternionf{0.0f, 0.0f, 0.0f, 2.0f});

        EXPECT_FLOAT_EQ(actor.GetRotation().Length(), 1.0f);
    }

    TEST(ActorTests, TheAnchorIsAddedToThePosition) {
        StillActor actor{ActorInput{.anchor = Vector3f{0.0f, 2.0f, 0.0f}, .position = Vector3f{1.0f, 0.0f, 0.0f}}};

        const Vector3f origin = WorldPointOf(actor, Vector3f::Zero());

        EXPECT_TRUE(Vector3f::NearlyEqual(origin, Vector3f{1.0f, 2.0f, 0.0f}));
    }

    TEST(ActorTests, TheAnchorIsNeitherRotatedNorScaled) {
        StillActor actor{ActorInput{
            .anchor = Vector3f{5.0f, 0.0f, 0.0f},
            .scale = Vector3f{2.0f, 2.0f, 2.0f},
            .rotation = Quaternionf{Vector3f::UnitY(), QuarterTurn}
        }};

        const Vector3f point = WorldPointOf(actor, Vector3f{1.0f, 0.0f, 0.0f});

        EXPECT_TRUE(Vector3f::NearlyEqual(point, Vector3f{5.0f, 0.0f, -2.0f}));
    }

    TEST(ActorTests, TheAnchorDoesNotChangeHowDirectionsAreRotatedAndScaled) {
        const ActorInput withoutAnchor{.scale = Vector3f{2.0f, 3.0f, 4.0f}, .rotation = Quaternionf{Vector3f::UnitX(), QuarterTurn}};
        ActorInput withAnchor = withoutAnchor;
        withAnchor.anchor = Vector3f{7.0f, -3.0f, 2.0f};
        StillActor plain{withoutAnchor};
        StillActor anchored{withAnchor};

        const Vector3f plainDirection = plain.GetWorldTransform().TransformVector(Vector3f{1.0f, 1.0f, 1.0f});
        const Vector3f anchoredDirection = anchored.GetWorldTransform().TransformVector(Vector3f{1.0f, 1.0f, 1.0f});

        EXPECT_TRUE(Vector3f::NearlyEqual(anchoredDirection, plainDirection));
    }

    TEST(ActorTests, ChangingThePositionUpdatesTheWorldTransform) {
        StillActor actor;
        static_cast<void>(actor.GetWorldTransform());

        actor.SetPosition(Vector3f{0.0f, 0.0f, 9.0f});

        EXPECT_TRUE(Vector3f::NearlyEqual(WorldPointOf(actor, Vector3f::Zero()), Vector3f{0.0f, 0.0f, 9.0f}));
    }

    TEST(ActorTests, TheDefaultForwardPointsDownNegativeZ) {
        StillActor actor;

        EXPECT_TRUE(Vector3f::NearlyEqual(actor.GetForward(), Vector3f{0.0f, 0.0f, -1.0f}));
    }

    TEST(ActorTests, TheForwardTurnsWithTheRotation) {
        StillActor actor{ActorInput{.rotation = Quaternionf{Vector3f::UnitY(), QuarterTurn}}};

        EXPECT_TRUE(Vector3f::NearlyEqual(actor.GetForward(), Vector3f{-1.0f, 0.0f, 0.0f}));
    }

    TEST(ActorTests, TheForwardStaysUnitLengthWhenScaled) {
        StillActor actor{ActorInput{.scale = Vector3f{5.0f, 5.0f, 5.0f}}};

        EXPECT_FLOAT_EQ(actor.GetForward().Length(), 1.0f);
    }

    TEST(ActorTests, AChildIsPlacedInsideItsParentSpace) {
        StillActor parent{ActorInput{.position = Vector3f{10.0f, 0.0f, 0.0f}, .rotation = Quaternionf{Vector3f::UnitY(), QuarterTurn}}};
        StillActor child{ActorInput{.position = Vector3f{1.0f, 0.0f, 0.0f}}};

        child.SetParent(&parent);

        EXPECT_TRUE(Vector3f::NearlyEqual(WorldPointOf(child, Vector3f::Zero()), Vector3f{10.0f, 0.0f, -1.0f}));
    }

    TEST(ActorTests, MovingTheParentMovesTheChild) {
        StillActor parent;
        StillActor child{ActorInput{.position = Vector3f{1.0f, 0.0f, 0.0f}}};
        child.SetParent(&parent);
        static_cast<void>(child.GetWorldTransform());

        parent.SetPosition(Vector3f{0.0f, 4.0f, 0.0f});

        EXPECT_TRUE(Vector3f::NearlyEqual(WorldPointOf(child, Vector3f::Zero()), Vector3f{1.0f, 4.0f, 0.0f}));
    }

    TEST(ActorTests, MovingTheGrandparentMovesTheGrandchild) {
        StillActor grandparent;
        StillActor parent;
        StillActor child;
        parent.SetParent(&grandparent);
        child.SetParent(&parent);
        static_cast<void>(child.GetWorldTransform());

        grandparent.SetPosition(Vector3f{3.0f, 0.0f, 0.0f});

        EXPECT_TRUE(Vector3f::NearlyEqual(WorldPointOf(child, Vector3f::Zero()), Vector3f{3.0f, 0.0f, 0.0f}));
    }

    TEST(ActorTests, AParentListsItsChildren) {
        StillActor parent;
        StillActor child;

        child.SetParent(&parent);

        ASSERT_EQ(parent.GetChildren().size(), 1u);
        EXPECT_EQ(parent.GetChildren().front(), &child);
    }

    TEST(ActorTests, ReparentingRemovesTheChildFromTheOldParent) {
        StillActor oldParent;
        StillActor newParent;
        StillActor child;
        child.SetParent(&oldParent);

        child.SetParent(&newParent);

        EXPECT_TRUE(oldParent.GetChildren().empty());
    }

    TEST(ActorTests, DetachingAChildBringsItBackToWorldSpace) {
        StillActor parent{ActorInput{.position = Vector3f{10.0f, 0.0f, 0.0f}}};
        StillActor child{ActorInput{.position = Vector3f{1.0f, 0.0f, 0.0f}}};
        child.SetParent(&parent);
        static_cast<void>(child.GetWorldTransform());

        child.SetParent(nullptr);

        EXPECT_TRUE(Vector3f::NearlyEqual(WorldPointOf(child, Vector3f::Zero()), Vector3f{1.0f, 0.0f, 0.0f}));
    }

    TEST(ActorTests, AnActorCannotBecomeTheChildOfItsOwnDescendant) {
        StillActor parent;
        StillActor child;
        child.SetParent(&parent);

        parent.SetParent(&child);

        EXPECT_EQ(parent.GetParent(), nullptr);
    }

    TEST(ActorTests, AnActorCannotBeItsOwnParent) {
        StillActor actor;

        actor.SetParent(&actor);

        EXPECT_EQ(actor.GetParent(), nullptr);
    }

    TEST(ActorTests, DestroyingTheParentLeavesTheChildWithoutAParent) {
        auto parent = std::make_unique<StillActor>();
        StillActor child;
        child.SetParent(parent.get());

        parent.reset();

        EXPECT_EQ(child.GetParent(), nullptr);
    }

    TEST(ActorTests, DestroyingAChildRemovesItFromTheParent) {
        StillActor parent;
        auto child = std::make_unique<StillActor>();
        child->SetParent(&parent);

        child.reset();

        EXPECT_TRUE(parent.GetChildren().empty());
    }

    TEST(ActorTests, AComponentAddedDuringAnUpdateJoinsTheActorAfterThatUpdate) {
        int spawnedUpdates = 0;
        SpawningActor actor{spawnedUpdates};

        actor.Update(0.016f);

        EXPECT_EQ(actor.GetComponents().size(), 1u);
    }

    TEST(ActorTests, AComponentAddedDuringAnUpdateStartsUpdatingOnTheNextUpdate) {
        int spawnedUpdates = 0;
        SpawningActor actor{spawnedUpdates};
        actor.Update(0.016f);

        actor.Update(0.016f);

        EXPECT_EQ(spawnedUpdates, 1);
    }

    TEST(ActorTests, ComponentsCanReachTheirActor) {
        int updates = 0;
        StillActor actor;
        auto component = std::make_unique<CountingComponent>(actor, updates);

        Actor* owner = component->GetActor();

        EXPECT_EQ(owner, &actor);
    }
}
