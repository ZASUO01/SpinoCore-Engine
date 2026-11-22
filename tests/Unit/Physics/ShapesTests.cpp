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
#include "SpinoCore/Physics/Ray.h"

namespace {
    using SpinoCore::Math::Quaternionf;
    using SpinoCore::Math::Vector3f;
    using SpinoCore::Physics::Capsule;
    using SpinoCore::Physics::Contact;
    using SpinoCore::Physics::Flipped;
    using SpinoCore::Physics::MakeOrientedBox;
    using SpinoCore::Physics::OrientedBox;
    using SpinoCore::Physics::Plane;
    using SpinoCore::Physics::Ray;
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
    TEST(ShapesTests, ARayTowardASphereHitsItsNearSurface) {
        const Ray ray{.origin = Vector3f{0.0f, 0.0f, 5.0f}, .direction = Vector3f::NegUnitZ()};

        const auto hit = Physics::IntersectRaySphere(ray, Sphere{.center = Vector3f::Zero(), .radius = 1.0f});

        ASSERT_TRUE(hit.has_value());
        EXPECT_NEAR(hit->distance, 4.0f, Tolerance);
        EXPECT_TRUE(Vector3f::NearlyEqual(hit->normal, Vector3f::UnitZ()));
    }

    TEST(ShapesTests, ARayPointingAwayFromASphereMissesIt) {
        const Ray ray{.origin = Vector3f{0.0f, 0.0f, 5.0f}, .direction = Vector3f::UnitZ()};

        const auto hit = Physics::IntersectRaySphere(ray, Sphere{.center = Vector3f::Zero(), .radius = 1.0f});

        EXPECT_FALSE(hit.has_value());
    }

    TEST(ShapesTests, ARayPassingBesideASphereMissesIt) {
        const Ray ray{.origin = Vector3f{2.0f, 0.0f, 5.0f}, .direction = Vector3f::NegUnitZ()};

        const auto hit = Physics::IntersectRaySphere(ray, Sphere{.center = Vector3f::Zero(), .radius = 1.0f});

        EXPECT_FALSE(hit.has_value());
    }

    TEST(ShapesTests, ARayStartingInsideASphereHitsItAtItsOrigin) {
        const Ray ray{.origin = Vector3f::Zero(), .direction = Vector3f::UnitX()};

        const auto hit = Physics::IntersectRaySphere(ray, Sphere{.center = Vector3f::Zero(), .radius = 1.0f});

        ASSERT_TRUE(hit.has_value());
        EXPECT_NEAR(hit->distance, 0.0f, Tolerance);
    }

    TEST(ShapesTests, ARayTowardABoxHitsTheFaceItMeetsFirst) {
        const Ray ray{.origin = Vector3f{-5.0f, 0.2f, 0.1f}, .direction = Vector3f::UnitX()};

        const auto hit = Physics::IntersectRayBox(ray, UnitBoxAt(Vector3f::Zero()));

        ASSERT_TRUE(hit.has_value());
        EXPECT_NEAR(hit->distance, 4.5f, Tolerance);
        EXPECT_TRUE(Vector3f::NearlyEqual(hit->normal, Vector3f{-1.0f, 0.0f, 0.0f}));
    }

    TEST(ShapesTests, ARayHitsARotatedBoxOnItsCorner) {
        const Quaternionf turn{Vector3f::UnitY(), SpinoCore::Math::Operations::ToRadians(45.0f)};
        const Ray ray{.origin = Vector3f{-5.0f, 0.0f, 0.0f}, .direction = Vector3f::UnitX()};

        const auto hit = Physics::IntersectRayBox(ray, UnitBoxAt(Vector3f::Zero(), turn));

        ASSERT_TRUE(hit.has_value());
        EXPECT_NEAR(hit->distance, 5.0f - std::sqrt(0.5f), Tolerance);
    }

    TEST(ShapesTests, ARayPassingAboveABoxMissesIt) {
        const Ray ray{.origin = Vector3f{-5.0f, 0.6f, 0.0f}, .direction = Vector3f::UnitX()};

        const auto hit = Physics::IntersectRayBox(ray, UnitBoxAt(Vector3f::Zero()));

        EXPECT_FALSE(hit.has_value());
    }

    TEST(ShapesTests, ARayTowardTheSideOfACapsuleHitsItsCylinder) {
        const Ray ray{.origin = Vector3f{5.0f, 0.5f, 0.0f}, .direction = Vector3f{-1.0f, 0.0f, 0.0f}};

        const auto hit = Physics::IntersectRayCapsule(ray, VerticalCapsuleAt(Vector3f::Zero()));

        ASSERT_TRUE(hit.has_value());
        EXPECT_NEAR(hit->distance, 4.5f, Tolerance);
        EXPECT_TRUE(Vector3f::NearlyEqual(hit->normal, Vector3f::UnitX()));
    }

    TEST(ShapesTests, ARayFromAboveACapsuleHitsItsTopCap) {
        const Ray ray{.origin = Vector3f{0.0f, 5.0f, 0.0f}, .direction = Vector3f{0.0f, -1.0f, 0.0f}};

        const auto hit = Physics::IntersectRayCapsule(ray, VerticalCapsuleAt(Vector3f::Zero()));

        ASSERT_TRUE(hit.has_value());
        EXPECT_NEAR(hit->distance, 3.5f, Tolerance);
        EXPECT_TRUE(Vector3f::NearlyEqual(hit->normal, Vector3f::UnitY()));
    }

    TEST(ShapesTests, ARayPassingBesideACapsuleMissesIt) {
        const Ray ray{.origin = Vector3f{5.0f, 0.0f, 0.8f}, .direction = Vector3f{-1.0f, 0.0f, 0.0f}};

        const auto hit = Physics::IntersectRayCapsule(ray, VerticalCapsuleAt(Vector3f::Zero()));

        EXPECT_FALSE(hit.has_value());
    }

    TEST(ShapesTests, ARayGoingDownHitsTheGround) {
        const Ray ray{.origin = Vector3f{1.0f, 3.0f, 2.0f}, .direction = Vector3f{0.0f, -1.0f, 0.0f}};

        const auto hit = Physics::IntersectRayPlane(ray, Ground);

        ASSERT_TRUE(hit.has_value());
        EXPECT_NEAR(hit->distance, 3.0f, Tolerance);
        EXPECT_TRUE(Vector3f::NearlyEqual(hit->normal, Vector3f::UnitY()));
    }

    TEST(ShapesTests, ARayParallelToTheGroundMissesIt) {
        const Ray ray{.origin = Vector3f{0.0f, 3.0f, 0.0f}, .direction = Vector3f::UnitX()};

        const auto hit = Physics::IntersectRayPlane(ray, Ground);

        EXPECT_FALSE(hit.has_value());
    }

    TEST(ShapesTests, ARayGoingUpMissesTheGroundBelowIt) {
        const Ray ray{.origin = Vector3f{0.0f, 3.0f, 0.0f}, .direction = Vector3f::UnitY()};

        const auto hit = Physics::IntersectRayPlane(ray, Ground);

        EXPECT_FALSE(hit.has_value());
    }
}
