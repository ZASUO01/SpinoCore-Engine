// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <gtest/gtest.h>
#include "MathTestHelpers.h"
#include "SpinoCore/Math/Math.h"
#include "SpinoCore/Math/MatrixFactory.h"
#include "SpinoCore/Math/Quaternion.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Math::Quaternionf;
    using SpinoCore::Math::Vector3f;
    using SpinoCore::Tests::QuaternionsNear;
    using SpinoCore::Tests::VectorsNear;
    namespace MatrixFactory = SpinoCore::Math::MatrixFactory;
    namespace Operations = SpinoCore::Math::Operations;

    constexpr float HalfPi = SpinoCore::Math::Constants::PIOver2<float>;
    constexpr float Pi = SpinoCore::Math::Constants::PI<float>;

    // The angle a unit quaternion rotates by, ignoring the q / -q ambiguity.
    [[nodiscard]] float AngleBetween(const Quaternionf& a, const Quaternionf& b) {
        return 2.0f * Operations::Acos(Operations::Abs(a.Dot(b)));
    }

    TEST(QuaternionTests, StartsAsTheIdentityRotation) {
        const Quaternionf identity;
        const Vector3f v{1.0f, 2.0f, 3.0f};

        EXPECT_TRUE(QuaternionsNear(identity, Quaternionf::Identity()));
        EXPECT_TRUE(VectorsNear(identity.Rotate(v), v));
    }

    TEST(QuaternionTests, AxisAngleRotatesCounterClockwiseAboutTheAxis) {
        const Quaternionf quarterTurnZ{Vector3f::UnitZ(), HalfPi};

        EXPECT_TRUE(VectorsNear(quarterTurnZ.Rotate(Vector3f::UnitX()), Vector3f::UnitY()));
        EXPECT_TRUE(VectorsNear(quarterTurnZ * Vector3f::UnitY(), Vector3f::NegUnitX()));
        EXPECT_FLOAT_EQ(quarterTurnZ.Length(), 1.0f);
    }

    TEST(QuaternionTests, ProductAppliesTheRightOperandFirst) {
        const Quaternionf yaw{Vector3f::UnitY(), HalfPi};
        const Quaternionf roll{Vector3f::UnitZ(), HalfPi};
        const Vector3f v = Vector3f::UnitX();

        EXPECT_TRUE(VectorsNear((yaw * roll).Rotate(v), yaw.Rotate(roll.Rotate(v))));
        EXPECT_FALSE(Vector3f::NearlyEqual((yaw * roll).Rotate(v), (roll * yaw).Rotate(v)));
    }

    TEST(QuaternionTests, CompoundProductMatchesTheBinaryOperator) {
        Quaternionf q{Vector3f::UnitX(), 0.3f};
        const Quaternionf other{Vector3f::UnitY(), -0.8f};
        const Quaternionf expected = q * other;

        q *= other;

        EXPECT_TRUE(QuaternionsNear(q, expected));
    }

    TEST(QuaternionTests, InverseUndoesTheRotation) {
        const Quaternionf q{Vector3f{1.0f, -1.0f, 2.0f}.Normalized(), 1.1f};
        bool success = false;

        const Quaternionf inverse = q.GetInverse(&success);

        ASSERT_TRUE(success);
        EXPECT_TRUE(QuaternionsNear(q * inverse, Quaternionf::Identity()));
        EXPECT_TRUE(VectorsNear(inverse.Rotate(q.Rotate(Vector3f{0.2f, 3.0f, -1.0f})), Vector3f{0.2f, 3.0f, -1.0f}));
    }

    TEST(QuaternionTests, InverseOfANonUnitQuaternionStillGivesIdentity) {
        const Quaternionf q{1.0f, 2.0f, 3.0f, 4.0f};

        EXPECT_TRUE(QuaternionsNear(q * q.GetInverse(), Quaternionf::Identity()));
    }

    TEST(QuaternionTests, AZeroQuaternionCannotBeInverted) {
        Quaternionf zero{0.0f, 0.0f, 0.0f, 0.0f};
        bool success = true;

        (void)zero.GetInverse(&success);
        zero.Invert();

        EXPECT_FALSE(success);
        EXPECT_FLOAT_EQ(zero.w, 0.0f);
    }

    TEST(QuaternionTests, ConjugateOfAUnitQuaternionIsItsInverse) {
        Quaternionf q{Vector3f::UnitY(), 0.9f};
        const Quaternionf conjugate = q.GetConjugate();
        q.Conjugate();

        EXPECT_TRUE(QuaternionsNear(conjugate, Quaternionf{Vector3f::UnitY(), 0.9f}.GetInverse()));
        EXPECT_TRUE(QuaternionsNear(q, conjugate));
    }

    TEST(QuaternionTests, NormalizeGivesUnitLengthAndLeavesZeroAlone) {
        Quaternionf q{1.0f, 2.0f, 3.0f, 4.0f};
        q.Normalize();
        Quaternionf zero{0.0f, 0.0f, 0.0f, 0.0f};
        zero.Normalize();

        EXPECT_FLOAT_EQ(q.Length(), 1.0f);
        EXPECT_FLOAT_EQ(zero.LengthSq(), 0.0f);
    }

    TEST(QuaternionTests, NearlyEqualTreatsQAndMinusQAsTheSameRotation) {
        const Quaternionf q{Vector3f::UnitX(), 0.5f};

        EXPECT_TRUE(Quaternionf::NearlyEqual(q, Quaternionf{-q.x, -q.y, -q.z, -q.w}));
        EXPECT_FALSE(Quaternionf::NearlyEqual(q, Quaternionf{Vector3f::UnitX(), 0.6f}));
    }

    TEST(QuaternionTests, SlerpHitsBothEnds) {
        const Quaternionf start{Vector3f::UnitY(), 0.2f};
        const Quaternionf end{Vector3f::UnitY(), 1.4f};

        EXPECT_TRUE(QuaternionsNear(Quaternionf::Slerp(start, end, 0.0f), start));
        EXPECT_TRUE(QuaternionsNear(Quaternionf::Slerp(start, end, 1.0f), end));
    }

    TEST(QuaternionTests, SlerpMovesAtAConstantAngularSpeed) {
        const Quaternionf start;
        const Quaternionf end{Vector3f::UnitZ(), 2.0f};

        EXPECT_TRUE(QuaternionsNear(Quaternionf::Slerp(start, end, 0.25f), Quaternionf{Vector3f::UnitZ(), 0.5f}));
        EXPECT_TRUE(QuaternionsNear(Quaternionf::Slerp(start, end, 0.5f), Quaternionf{Vector3f::UnitZ(), 1.0f}));
        EXPECT_NEAR(Quaternionf::Slerp(start, end, 0.5f).Length(), 1.0f, 1e-5f);
    }

    TEST(QuaternionTests, SlerpTakesTheShortestPath) {
        const Quaternionf start;
        // 300 degrees one way is 60 degrees the other way.
        const Quaternionf end{Vector3f::UnitZ(), Operations::ToRadians(300.0f)};

        const Quaternionf halfway = Quaternionf::Slerp(start, end, 0.5f);

        EXPECT_NEAR(AngleBetween(start, halfway), Operations::ToRadians(30.0f), 1e-4f);
    }

    TEST(QuaternionTests, SlerpBetweenNearlyEqualRotationsStaysNormalized) {
        const Quaternionf start{Vector3f::UnitX(), 0.5f};
        const Quaternionf end{Vector3f::UnitX(), 0.5001f};

        const Quaternionf result = Quaternionf::Slerp(start, end, 0.5f);

        EXPECT_NEAR(result.Length(), 1.0f, 1e-5f);
        EXPECT_TRUE(QuaternionsNear(result, Quaternionf{Vector3f::UnitX(), 0.50005f}));
    }

    TEST(QuaternionTests, LerpReturnsAUnitQuaternionOnTheShortPath) {
        const Quaternionf start;
        const Quaternionf end{Vector3f::UnitZ(), Operations::ToRadians(300.0f)};

        const Quaternionf result = Quaternionf::Lerp(start, end, 0.5f);

        EXPECT_NEAR(result.Length(), 1.0f, 1e-5f);
        EXPECT_NEAR(AngleBetween(start, result), Operations::ToRadians(30.0f), 1e-4f);
    }

    TEST(QuaternionTests, FromEulerWithASingleAngleMatchesTheAxisRotation) {
        EXPECT_TRUE(QuaternionsNear(Quaternionf::FromEuler(0.7f, 0.0f, 0.0f), Quaternionf{Vector3f::UnitX(), 0.7f}));
        EXPECT_TRUE(QuaternionsNear(Quaternionf::FromEuler(0.0f, 0.7f, 0.0f), Quaternionf{Vector3f::UnitY(), 0.7f}));
        EXPECT_TRUE(QuaternionsNear(Quaternionf::FromEuler(0.0f, 0.0f, 0.7f), Quaternionf{Vector3f::UnitZ(), 0.7f}));
    }

    TEST(QuaternionTests, FromEulerAppliesRollThenPitchThenYaw) {
        const float pitch = 0.3f;
        const float yaw = 1.2f;
        const float roll = -0.5f;
        const Quaternionf expected = Quaternionf{Vector3f::UnitY(), yaw} * Quaternionf{Vector3f::UnitX(), pitch} *
                                     Quaternionf{Vector3f::UnitZ(), roll};

        EXPECT_TRUE(QuaternionsNear(Quaternionf::FromEuler(pitch, yaw, roll), expected));
    }

    TEST(QuaternionTests, FromAxesRebuildsTheRotationOfAMatrix) {
        // Cover each branch of the conversion: positive trace and each dominant diagonal.
        const Quaternionf rotations[] = {
            Quaternionf{Vector3f{1.0f, 1.0f, 0.0f}.Normalized(), 0.4f},
            Quaternionf{Vector3f::UnitX(), Pi * 0.95f},
            Quaternionf{Vector3f::UnitY(), Pi * 0.95f},
            Quaternionf{Vector3f::UnitZ(), Pi * 0.95f},
        };

        for (const Quaternionf& rotation : rotations) {
            const auto matrix = MatrixFactory::CreateFromQuaternion(rotation);
            const Quaternionf rebuilt = Quaternionf::FromAxes(
                MatrixFactory::GetRight(matrix), MatrixFactory::GetUp(matrix), -MatrixFactory::GetForward(matrix));

            EXPECT_TRUE(QuaternionsNear(rebuilt, rotation));
        }
    }

    TEST(QuaternionTests, LookRotationTurnsTheDefaultForwardTowardTheTarget) {
        const Vector3f forward = Vector3f{1.0f, 0.5f, -2.0f}.Normalized();

        const Quaternionf look = Quaternionf::LookRotation(forward);

        EXPECT_TRUE(VectorsNear(look.Rotate(Vector3f::NegUnitZ()), forward));
        // The rotated up stays in the plane of world up and forward, so the view does not roll.
        EXPECT_NEAR(look.Rotate(Vector3f::UnitX()).y, 0.0f, 1e-5f);
    }

    TEST(QuaternionTests, LookRotationAlongNegativeZIsIdentity) {
        EXPECT_TRUE(QuaternionsNear(Quaternionf::LookRotation(Vector3f::NegUnitZ()), Quaternionf::Identity()));
    }

    TEST(QuaternionTests, LookRotationStraightUpStillPointsUp) {
        const Quaternionf look = Quaternionf::LookRotation(Vector3f::UnitY());

        EXPECT_NEAR(look.Length(), 1.0f, 1e-5f);
        EXPECT_TRUE(VectorsNear(look.Rotate(Vector3f::NegUnitZ()), Vector3f::UnitY()));
    }

    TEST(QuaternionTests, LookRotationWithAZeroForwardIsIdentity) {
        EXPECT_TRUE(QuaternionsNear(Quaternionf::LookRotation(Vector3f::Zero()), Quaternionf::Identity()));
    }
}
