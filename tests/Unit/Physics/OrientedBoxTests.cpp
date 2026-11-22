// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <optional>
#include <gtest/gtest.h>
#include "Physics/OrientedBox.h"
#include "SpinoCore/Math/Math.h"
#include "SpinoCore/Math/MatrixFactory.h"
#include "SpinoCore/Math/Quaternion.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Math::Quaternionf;
    using SpinoCore::Math::Vector3f;
    using SpinoCore::Physics::Contact;
    using SpinoCore::Physics::MakeOrientedBox;
    using SpinoCore::Physics::OrientedBox;
    using SpinoCore::Physics::TestOrientedBoxes;
    namespace MatrixFactory = SpinoCore::Math::MatrixFactory;

    constexpr float EighthTurn = SpinoCore::Math::Constants::PIOver4<float>;
    const Vector3f UnitHalfExtents{0.5f, 0.5f, 0.5f};

    [[nodiscard]] OrientedBox UnitBoxAt(const Vector3f& position, const Quaternionf& rotation = Quaternionf::Identity()) {
        return MakeOrientedBox(MatrixFactory::CreateTransform(position, rotation, Vector3f::One()), UnitHalfExtents);
    }

    TEST(OrientedBoxTests, TheCenterComesFromTheTranslation) {
        const OrientedBox box = UnitBoxAt(Vector3f{1.0f, 2.0f, 3.0f});

        EXPECT_TRUE(Vector3f::NearlyEqual(box.center, Vector3f{1.0f, 2.0f, 3.0f}));
    }

    TEST(OrientedBoxTests, TheScaleEnlargesTheHalfExtents) {
        const OrientedBox box = MakeOrientedBox(MatrixFactory::CreateScale(Vector3f{3.0f, 1.0f, 2.0f}), UnitHalfExtents);

        EXPECT_TRUE(Vector3f::NearlyEqual(box.halfExtents, Vector3f{1.5f, 0.5f, 1.0f}));
    }

    TEST(OrientedBoxTests, TheAxesFollowTheRotation) {
        const OrientedBox box = UnitBoxAt(Vector3f::Zero(), Quaternionf{Vector3f::UnitZ(), EighthTurn * 2.0f});

        EXPECT_TRUE(Vector3f::NearlyEqual(box.axes[0], Vector3f::UnitY()));
    }

    TEST(OrientedBoxTests, SeparatedBoxesDoNotCollide) {
        const auto contact = TestOrientedBoxes(UnitBoxAt(Vector3f::Zero()), UnitBoxAt(Vector3f{2.0f, 0.0f, 0.0f}));

        EXPECT_FALSE(contact.has_value());
    }

    TEST(OrientedBoxTests, OverlappingBoxesReportThePenetrationDepth) {
        const auto contact = TestOrientedBoxes(UnitBoxAt(Vector3f::Zero()), UnitBoxAt(Vector3f{0.8f, 0.0f, 0.0f}));

        ASSERT_TRUE(contact.has_value());
        EXPECT_NEAR(contact->depth, 0.2f, 1e-5f);
    }

    TEST(OrientedBoxTests, TheNormalPointsFromTheFirstBoxToTheSecond) {
        const auto contact = TestOrientedBoxes(UnitBoxAt(Vector3f::Zero()), UnitBoxAt(Vector3f{0.8f, 0.1f, 0.0f}));

        ASSERT_TRUE(contact.has_value());
        EXPECT_TRUE(Vector3f::NearlyEqual(contact->normal, Vector3f::UnitX()));
    }

    TEST(OrientedBoxTests, TheNormalFlipsWhenTheSecondBoxIsOnTheOtherSide) {
        const auto contact = TestOrientedBoxes(UnitBoxAt(Vector3f::Zero()), UnitBoxAt(Vector3f{0.0f, -0.9f, 0.0f}));

        ASSERT_TRUE(contact.has_value());
        EXPECT_TRUE(Vector3f::NearlyEqual(contact->normal, Vector3f::NegUnitY()));
    }

    TEST(OrientedBoxTests, AnAxisAlignedBoxJustOutOfReachDoesNotCollide) {
        const auto contact = TestOrientedBoxes(UnitBoxAt(Vector3f::Zero()), UnitBoxAt(Vector3f{1.05f, 0.0f, 0.0f}));

        EXPECT_FALSE(contact.has_value());
    }

    TEST(OrientedBoxTests, TheSameBoxRotatedReachesWithItsCorner) {
        const Quaternionf rotated{Vector3f::UnitZ(), EighthTurn};

        const auto contact = TestOrientedBoxes(UnitBoxAt(Vector3f::Zero()), UnitBoxAt(Vector3f{1.05f, 0.0f, 0.0f}, rotated));

        EXPECT_TRUE(contact.has_value());
    }

    TEST(OrientedBoxTests, RotatedBoxesWhoseBoundsOverlapCanStillBeApart) {
        const Quaternionf rotated{Vector3f::UnitZ(), EighthTurn};

        const auto contact = TestOrientedBoxes(UnitBoxAt(Vector3f::Zero()), UnitBoxAt(Vector3f{0.9f, 0.9f, 0.0f}, rotated));

        EXPECT_FALSE(contact.has_value());
    }
}
