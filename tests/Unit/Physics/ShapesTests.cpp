// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <cmath>
#include <optional>
#include <gtest/gtest.h>
#include "Physics/OrientedBox.h"
#include "Physics/Shapes.h"
#include "SpinoCore/Math/Math.h"
#include "SpinoCore/Math/MatrixFactory.h"
#include "SpinoCore/Math/Quaternion.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Math::Quaternionf;
    using SpinoCore::Math::Vector3f;
    using SpinoCore::Physics::Capsule;
    using SpinoCore::Physics::Contact;
    using SpinoCore::Physics::Flipped;
    using SpinoCore::Physics::MakeOrientedBox;
    using SpinoCore::Physics::OrientedBox;
    using SpinoCore::Physics::Plane;
    using SpinoCore::Physics::Sphere;
    namespace Physics = SpinoCore::Physics;
    namespace MatrixFactory = SpinoCore::Math::MatrixFactory;

    constexpr float Tolerance = 1e-4f;
    const Plane Ground{.normal = Vector3f::UnitY(), .distance = 0.0f};

    [[nodiscard]] OrientedBox UnitBoxAt(const Vector3f& position, const Quaternionf& rotation = Quaternionf::Identity()) {
        return MakeOrientedBox(MatrixFactory::CreateTransform(position, rotation, Vector3f::One()), Vector3f{0.5f, 0.5f, 0.5f});
    }

    [[nodiscard]] Capsule VerticalCapsuleAt(const Vector3f& center) {
        return Capsule{.start = center - Vector3f{0.0f, 1.0f, 0.0f}, .end = center + Vector3f{0.0f, 1.0f, 0.0f}, .radius = 0.5f};
    }

    TEST(ShapesTests, SpheresApartDoNotCollide) {
        const auto contact = Physics::TestSpheres(Sphere{.center = Vector3f::Zero(), .radius = 0.5f}, Sphere{.center = Vector3f{2.0f, 0.0f, 0.0f}, .radius = 0.5f});

        EXPECT_FALSE(contact.has_value());
    }

    TEST(ShapesTests, OverlappingSpheresPushAlongTheLineBetweenCenters) {
        const auto contact = Physics::TestSpheres(Sphere{.center = Vector3f::Zero(), .radius = 0.5f}, Sphere{.center = Vector3f{0.0f, 0.8f, 0.0f}, .radius = 0.5f});

        ASSERT_TRUE(contact.has_value());
        EXPECT_TRUE(Vector3f::NearlyEqual(contact->normal, Vector3f::UnitY()));
        EXPECT_NEAR(contact->depth, 0.2f, Tolerance);
    }

    TEST(ShapesTests, SpheresWithTheSameCenterGetAValidNormal) {
        const auto contact = Physics::TestSpheres(Sphere{.center = Vector3f::Zero(), .radius = 0.5f}, Sphere{.center = Vector3f::Zero(), .radius = 0.5f});

        ASSERT_TRUE(contact.has_value());
        EXPECT_FALSE(std::isnan(contact->normal.x));
        EXPECT_NEAR(contact->depth, 1.0f, Tolerance);
    }

    TEST(ShapesTests, ASphereBesideACapsuleIsPushedSideways) {
        const auto contact = Physics::TestSphereCapsule(Sphere{.center = Vector3f{0.8f, 0.3f, 0.0f}, .radius = 0.5f}, VerticalCapsuleAt(Vector3f::Zero()));

        ASSERT_TRUE(contact.has_value());
        EXPECT_TRUE(Vector3f::NearlyEqual(contact->normal, Vector3f::NegUnitX()));
        EXPECT_NEAR(contact->depth, 0.2f, Tolerance);
    }

    TEST(ShapesTests, ASphereAboveACapsuleReachesItsRoundedEnd) {
        const auto contact = Physics::TestSphereCapsule(Sphere{.center = Vector3f{0.0f, 1.8f, 0.0f}, .radius = 0.5f}, VerticalCapsuleAt(Vector3f::Zero()));

        ASSERT_TRUE(contact.has_value());
        EXPECT_NEAR(contact->depth, 0.2f, Tolerance);
    }

    TEST(ShapesTests, ParallelCapsulesSideBySideCollide) {
        const auto contact = Physics::TestCapsules(VerticalCapsuleAt(Vector3f::Zero()), VerticalCapsuleAt(Vector3f{0.9f, 0.5f, 0.0f}));

        ASSERT_TRUE(contact.has_value());
        EXPECT_TRUE(Vector3f::NearlyEqual(contact->normal, Vector3f::UnitX()));
        EXPECT_NEAR(contact->depth, 0.1f, Tolerance);
    }

    TEST(ShapesTests, CrossedCapsulesTouchAtTheirClosestPoints) {
        const Capsule horizontal{.start = Vector3f{-1.0f, 0.0f, 0.8f}, .end = Vector3f{1.0f, 0.0f, 0.8f}, .radius = 0.5f};

        const auto contact = Physics::TestCapsules(VerticalCapsuleAt(Vector3f::Zero()), horizontal);

        ASSERT_TRUE(contact.has_value());
        EXPECT_TRUE(Vector3f::NearlyEqual(contact->normal, Vector3f::UnitZ()));
        EXPECT_NEAR(contact->depth, 0.2f, Tolerance);
    }

    TEST(ShapesTests, CapsulesApartDoNotCollide) {
        const auto contact = Physics::TestCapsules(VerticalCapsuleAt(Vector3f::Zero()), VerticalCapsuleAt(Vector3f{3.0f, 0.0f, 0.0f}));

        EXPECT_FALSE(contact.has_value());
    }

    TEST(ShapesTests, ASphereTouchingABoxFaceIsPushedAlongThatFace) {
        const auto contact = Physics::TestBoxSphere(UnitBoxAt(Vector3f::Zero()), Sphere{.center = Vector3f{0.0f, 0.0f, 0.9f}, .radius = 0.5f});

        ASSERT_TRUE(contact.has_value());
        EXPECT_TRUE(Vector3f::NearlyEqual(contact->normal, Vector3f::UnitZ()));
        EXPECT_NEAR(contact->depth, 0.1f, Tolerance);
    }

    TEST(ShapesTests, ASphereNearABoxCornerButOutsideDoesNotCollide) {
        const auto contact = Physics::TestBoxSphere(UnitBoxAt(Vector3f::Zero()), Sphere{.center = Vector3f{0.9f, 0.9f, 0.0f}, .radius = 0.5f});

        EXPECT_FALSE(contact.has_value());
    }

    TEST(ShapesTests, ASphereCenteredInsideABoxLeavesThroughTheNearestFace) {
        const auto contact = Physics::TestBoxSphere(UnitBoxAt(Vector3f::Zero()), Sphere{.center = Vector3f{0.0f, -0.4f, 0.0f}, .radius = 0.2f});

        ASSERT_TRUE(contact.has_value());
        EXPECT_TRUE(Vector3f::NearlyEqual(contact->normal, Vector3f::NegUnitY()));
        EXPECT_NEAR(contact->depth, 0.3f, Tolerance);
    }

    TEST(ShapesTests, ACapsuleLyingOnABoxIsPushedUp) {
        const Capsule lying{.start = Vector3f{-1.0f, 0.9f, 0.0f}, .end = Vector3f{1.0f, 0.9f, 0.0f}, .radius = 0.5f};

        const auto contact = Physics::TestBoxCapsule(UnitBoxAt(Vector3f::Zero()), lying);

        ASSERT_TRUE(contact.has_value());
        EXPECT_TRUE(Vector3f::NearlyEqual(contact->normal, Vector3f::UnitY()));
        EXPECT_NEAR(contact->depth, 0.1f, Tolerance);
    }

    TEST(ShapesTests, ACapsuleBesideABoxDoesNotCollide) {
        const auto contact = Physics::TestBoxCapsule(UnitBoxAt(Vector3f::Zero()), VerticalCapsuleAt(Vector3f{1.2f, 0.0f, 0.0f}));

        EXPECT_FALSE(contact.has_value());
    }

    TEST(ShapesTests, ABoxSinkingIntoAPlaneIsPushedOut) {
        const auto contact = Physics::TestBoxPlane(UnitBoxAt(Vector3f{0.0f, 0.4f, 0.0f}), Ground);

        ASSERT_TRUE(contact.has_value());
        EXPECT_TRUE(Vector3f::NearlyEqual(contact->normal, Vector3f::NegUnitY()));
        EXPECT_NEAR(contact->depth, 0.1f, Tolerance);
    }

    TEST(ShapesTests, ATiltedBoxReachesAPlaneWithItsCorner) {
        const Quaternionf tilted{Vector3f::UnitZ(), SpinoCore::Math::Constants::PIOver4<float>};

        const auto contact = Physics::TestBoxPlane(UnitBoxAt(Vector3f{0.0f, 0.6f, 0.0f}, tilted), Ground);

        EXPECT_TRUE(contact.has_value());
    }

    TEST(ShapesTests, ASphereAboveAPlaneDoesNotCollide) {
        const auto contact = Physics::TestSpherePlane(Sphere{.center = Vector3f{0.0f, 1.0f, 0.0f}, .radius = 0.5f}, Ground);

        EXPECT_FALSE(contact.has_value());
    }

    TEST(ShapesTests, ASphereSinkingIntoAPlaneIsPushedOut) {
        const auto contact = Physics::TestSpherePlane(Sphere{.center = Vector3f{0.0f, 0.3f, 0.0f}, .radius = 0.5f}, Ground);

        ASSERT_TRUE(contact.has_value());
        EXPECT_TRUE(Vector3f::NearlyEqual(contact->normal, Vector3f::NegUnitY()));
        EXPECT_NEAR(contact->depth, 0.2f, Tolerance);
    }

    TEST(ShapesTests, ACapsuleTouchesAPlaneWithItsLowestEnd) {
        const auto contact = Physics::TestCapsulePlane(VerticalCapsuleAt(Vector3f{0.0f, 1.4f, 0.0f}), Ground);

        ASSERT_TRUE(contact.has_value());
        EXPECT_NEAR(contact->depth, 0.1f, Tolerance);
    }

    TEST(ShapesTests, FlippingAContactReversesItsNormal) {
        const std::optional<Contact> contact = Contact{.normal = Vector3f::UnitX(), .depth = 0.3f};

        const auto flipped = Flipped(contact);

        ASSERT_TRUE(flipped.has_value());
        EXPECT_TRUE(Vector3f::NearlyEqual(flipped->normal, Vector3f::NegUnitX()));
        EXPECT_NEAR(flipped->depth, 0.3f, Tolerance);
    }
}
