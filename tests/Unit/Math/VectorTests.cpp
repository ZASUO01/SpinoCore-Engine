// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <gtest/gtest.h>
#include "MathTestHelpers.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Math::Vector2f;
    using SpinoCore::Math::Vector2i;
    using SpinoCore::Math::Vector3f;
    using SpinoCore::Math::Vector4f;
    using SpinoCore::Tests::VectorsNear;

    TEST(VectorTests, StartAtZero) {
        const Vector3f vector;

        EXPECT_TRUE(VectorsNear(vector, Vector3f::Zero()));
        EXPECT_EQ(Vector2i{}.x, 0);
        EXPECT_FLOAT_EQ(Vector4f{}.w, 0.0f);
    }

    TEST(VectorTests, ComponentsAreContiguousSoTheyCanBeSentToTheGPU) {
        Vector4f vector{1.0f, 2.0f, 3.0f, 4.0f};

        const float* data = vector.Data();

        EXPECT_FLOAT_EQ(data[0], 1.0f);
        EXPECT_FLOAT_EQ(data[3], 4.0f);
    }

    TEST(VectorTests, ArithmeticIsComponentWise) {
        const Vector3f a{1.0f, 2.0f, 3.0f};
        const Vector3f b{4.0f, 5.0f, 6.0f};

        EXPECT_TRUE(VectorsNear(a + b, Vector3f{5.0f, 7.0f, 9.0f}));
        EXPECT_TRUE(VectorsNear(b - a, Vector3f{3.0f, 3.0f, 3.0f}));
        EXPECT_TRUE(VectorsNear(a * b, Vector3f{4.0f, 10.0f, 18.0f}));
        EXPECT_TRUE(VectorsNear(b / a, Vector3f{4.0f, 2.5f, 2.0f}));
        EXPECT_TRUE(VectorsNear(2.0f * a, a * 2.0f));
        EXPECT_TRUE(VectorsNear(a / 2.0f, Vector3f{0.5f, 1.0f, 1.5f}));
        EXPECT_TRUE(VectorsNear(6.0f / a, Vector3f{6.0f, 3.0f, 2.0f}));
        EXPECT_TRUE(VectorsNear(-a, Vector3f{-1.0f, -2.0f, -3.0f}));
    }

    TEST(VectorTests, IntegerVectorsSupportArithmetic) {
        const Vector2i sum = Vector2i{1, 2} + Vector2i{3, 4} * 2;

        EXPECT_EQ(sum.x, 7);
        EXPECT_EQ(sum.y, 10);
        EXPECT_EQ(Vector2i(3, 4).LengthSq(), 25);
    }

    TEST(VectorTests, LengthAndDistance) {
        EXPECT_FLOAT_EQ((Vector3f{2.0f, 3.0f, 6.0f}).Length(), 7.0f);
        EXPECT_FLOAT_EQ((Vector2f{0.0f, 0.0f}).Distance(Vector2f{3.0f, 4.0f}), 5.0f);
        EXPECT_FLOAT_EQ((Vector4f{1.0f, 1.0f, 1.0f, 1.0f}).Length(), 2.0f);
    }

    TEST(VectorTests, NormalizedHasUnitLengthAndKeepsTheDirection) {
        const Vector3f normalized = Vector3f{0.0f, 3.0f, 4.0f}.Normalized();

        EXPECT_FLOAT_EQ(normalized.Length(), 1.0f);
        EXPECT_TRUE(VectorsNear(normalized, Vector3f{0.0f, 0.6f, 0.8f}));
    }

    TEST(VectorTests, NormalizingAZeroVectorLeavesItZeroInsteadOfNaN) {
        Vector3f zero;
        zero.Normalize();

        EXPECT_TRUE(VectorsNear(zero, Vector3f::Zero()));
        EXPECT_TRUE(VectorsNear(Vector3f::Zero().Normalized(), Vector3f::Zero()));
        EXPECT_FLOAT_EQ(Vector2f{}.Normalized().x, 0.0f);
    }

    TEST(VectorTests, CrossProductIsRightHanded) {
        EXPECT_TRUE(VectorsNear(Vector3f::UnitX().Cross(Vector3f::UnitY()), Vector3f::UnitZ()));
        EXPECT_TRUE(VectorsNear(Vector3f::UnitY().Cross(Vector3f::UnitZ()), Vector3f::UnitX()));
        EXPECT_TRUE(VectorsNear(Vector3f::UnitY().Cross(Vector3f::UnitX()), Vector3f::NegUnitZ()));
    }

    TEST(VectorTests, TwoDimensionalCrossIsPositiveForACounterClockwiseTurn) {
        EXPECT_FLOAT_EQ(Vector2f::UnitX().Cross(Vector2f::UnitY()), 1.0f);
        EXPECT_FLOAT_EQ(Vector2f::UnitY().Cross(Vector2f::UnitX()), -1.0f);
    }

    TEST(VectorTests, DotProduct) {
        EXPECT_FLOAT_EQ((Vector3f{1.0f, 2.0f, 3.0f}).Dot(Vector3f{4.0f, -5.0f, 6.0f}), 12.0f);
        EXPECT_FLOAT_EQ(Vector3f::UnitX().Dot(Vector3f::UnitY()), 0.0f);
    }

    TEST(VectorTests, ProjectionAndRejectionSplitAVectorIntoParallelAndPerpendicularParts) {
        const Vector3f u{3.0f, 4.0f, 0.0f};
        const Vector3f onto{2.0f, 0.0f, 0.0f};

        const Vector3f parallel = Vector3f::Project(u, onto);
        const Vector3f perpendicular = Vector3f::Rejection(u, onto);

        EXPECT_TRUE(VectorsNear(parallel, Vector3f{3.0f, 0.0f, 0.0f}));
        EXPECT_TRUE(VectorsNear(perpendicular, Vector3f{0.0f, 4.0f, 0.0f}));
        EXPECT_TRUE(VectorsNear(parallel + perpendicular, u));
    }

    TEST(VectorTests, ProjectingOntoAZeroVectorGivesZero) {
        EXPECT_TRUE(VectorsNear(Vector3f::Project(Vector3f{1.0f, 2.0f, 3.0f}, Vector3f::Zero()), Vector3f::Zero()));
    }

    TEST(VectorTests, LerpMovesLinearlyBetweenTheEnds) {
        const Vector3f start{0.0f, 10.0f, -2.0f};
        const Vector3f end{10.0f, 20.0f, 2.0f};

        EXPECT_TRUE(VectorsNear(Vector3f::Lerp(start, end, 0.0f), start));
        EXPECT_TRUE(VectorsNear(Vector3f::Lerp(start, end, 1.0f), end));
        EXPECT_TRUE(VectorsNear(Vector3f::Lerp(start, end, 0.25f), Vector3f{2.5f, 12.5f, -1.0f}));
    }

    TEST(VectorTests, ReflectBouncesOffTheSurfaceNormal) {
        const Vector3f incoming{1.0f, -1.0f, 0.0f};

        EXPECT_TRUE(VectorsNear(Vector3f::Reflect(incoming, Vector3f::UnitY()), Vector3f{1.0f, 1.0f, 0.0f}));
    }

    TEST(VectorTests, Vector4CanBeBuiltFromAVector3) {
        const Vector4f point{Vector3f{1.0f, 2.0f, 3.0f}, 1.0f};

        EXPECT_TRUE(Vector4f::NearlyEqual(point, Vector4f{1.0f, 2.0f, 3.0f, 1.0f}));
    }
}
