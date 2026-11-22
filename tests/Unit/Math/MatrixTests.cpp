// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <cstddef>
#include <gtest/gtest.h>
#include "MathTestHelpers.h"
#include "SpinoCore/Math/Math.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/MatrixFactory.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Math::Matrix3f;
    using SpinoCore::Math::Matrix4f;
    using SpinoCore::Math::Vector3f;
    using SpinoCore::Math::Vector4f;
    using SpinoCore::Tests::MatricesNear;
    using SpinoCore::Tests::VectorsNear;
    namespace MatrixFactory = SpinoCore::Math::MatrixFactory;

    [[nodiscard]] Matrix4f GeneralMatrix() {
        constexpr float values[16] = {
            2.0f, 1.0f, 0.0f, 0.5f,
            -1.0f, 3.0f, 2.0f, 0.0f,
            0.0f, 1.0f, 4.0f, -1.0f,
            1.0f, -2.0f, 0.5f, 3.0f
        };
        return Matrix4f{values};
    }

    [[nodiscard]] Matrix3f General3x3() {
        constexpr float values[9] = {
            2.0f, 0.0f, 1.0f,
            1.0f, 3.0f, -1.0f,
            0.5f, 1.0f, 4.0f
        };
        return Matrix3f{values};
    }

    [[nodiscard]] bool IsIdentity(const Matrix3f& matrix) {
        for (std::size_t row = 0; row < 3; ++row) {
            for (std::size_t col = 0; col < 3; ++col) {
                const float expected = row == col ? 1.0f : 0.0f;
                if (!SpinoCore::Math::Operations::NearlyEqual(matrix(row, col), expected, 1e-5f)) return false;
            }
        }
        return true;
    }

    TEST(MatrixTests, StartsAsIdentity) {
        EXPECT_TRUE(MatricesNear(Matrix4f{}, Matrix4f::Identity()));
        EXPECT_FLOAT_EQ(Matrix4f{}(2, 2), 1.0f);
        EXPECT_FLOAT_EQ(Matrix4f{}(2, 1), 0.0f);
        EXPECT_TRUE(IsIdentity(Matrix3f{}));
    }

    TEST(MatrixTests, StorageIsColumnMajorLikeOpenGL) {
        Matrix4f matrix;
        matrix(1, 3) = 7.0f;

        EXPECT_FLOAT_EQ(matrix.Data()[13], 7.0f);
        EXPECT_FLOAT_EQ(MatrixFactory::CreateTranslation(Vector3f{4.0f, 5.0f, 6.0f}).Data()[12], 4.0f);
    }

    TEST(MatrixTests, MultiplicationAppliesTheRightOperandFirst) {
        const Matrix4f translate = MatrixFactory::CreateTranslation(Vector3f{10.0f, 0.0f, 0.0f});
        const Matrix4f scale = MatrixFactory::CreateScale(2.0f);
        const Vector3f point{1.0f, 1.0f, 1.0f};

        EXPECT_TRUE(VectorsNear((translate * scale).TransformPoint(point), Vector3f{12.0f, 2.0f, 2.0f}));
        EXPECT_TRUE(VectorsNear((scale * translate).TransformPoint(point), Vector3f{22.0f, 2.0f, 2.0f}));
    }

    TEST(MatrixTests, MultiplyingByIdentityChangesNothing) {
        const Matrix4f matrix = GeneralMatrix();

        EXPECT_TRUE(MatricesNear(matrix * Matrix4f::Identity(), matrix));
        EXPECT_TRUE(MatricesNear(Matrix4f::Identity() * matrix, matrix));
    }

    TEST(MatrixTests, CompoundMultiplicationMatchesTheBinaryOperator) {
        Matrix4f matrix = GeneralMatrix();
        const Matrix4f other = MatrixFactory::CreateRotationY(0.3f);

        matrix *= other;

        EXPECT_TRUE(MatricesNear(matrix, GeneralMatrix() * other));
    }

    TEST(MatrixTests, AMatrixTimesItsInverseIsIdentity) {
        const Matrix4f matrix = GeneralMatrix();
        bool success = false;

        const Matrix4f inverse = matrix.GetInverse(&success);

        ASSERT_TRUE(success);
        EXPECT_TRUE(MatricesNear(matrix * inverse, Matrix4f::Identity()));
        EXPECT_TRUE(MatricesNear(inverse * matrix, Matrix4f::Identity()));
    }

    TEST(MatrixTests, InvertingATransformUndoesIt) {
        const Matrix4f transform = MatrixFactory::CreateTranslation(Vector3f{1.0f, -2.0f, 3.0f}) *
                                   MatrixFactory::CreateRotationZ(0.7f) * MatrixFactory::CreateScale(2.0f);
        Matrix4f inverse = transform;
        inverse.Invert();
        const Vector3f point{0.5f, 4.0f, -1.0f};

        EXPECT_TRUE(VectorsNear(inverse.TransformPoint(transform.TransformPoint(point)), point));
    }

    TEST(MatrixTests, ASingularMatrixReportsFailureAndInvertLeavesItUntouched) {
        Matrix4f singular = MatrixFactory::CreateScale(1.0f, 0.0f, 1.0f);
        bool success = true;

        const Matrix4f inverse = singular.GetInverse(&success);
        singular.Invert();

        EXPECT_FALSE(success);
        EXPECT_TRUE(MatricesNear(inverse, Matrix4f::Identity()));
        EXPECT_TRUE(MatricesNear(singular, MatrixFactory::CreateScale(1.0f, 0.0f, 1.0f)));
    }

    TEST(MatrixTests, Matrix3TimesItsInverseIsIdentity) {
        const Matrix3f matrix = General3x3();
        bool success = false;

        const Matrix3f inverse = matrix.GetInverse(&success);

        ASSERT_TRUE(success);
        EXPECT_TRUE(IsIdentity(matrix * inverse));
        EXPECT_TRUE(IsIdentity(inverse * matrix));
    }

    TEST(MatrixTests, ASingularMatrix3ReportsFailure) {
        constexpr float values[9] = {1.0f, 2.0f, 3.0f, 2.0f, 4.0f, 6.0f, 0.0f, 1.0f, 1.0f};
        Matrix3f singular{values};
        bool success = true;

        (void)singular.GetInverse(&success);
        singular.Invert();

        EXPECT_FALSE(success);
        EXPECT_FLOAT_EQ(singular(1, 0), 2.0f);
    }

    TEST(MatrixTests, TransposeSwapsRowsAndColumns) {
        const Matrix4f matrix = GeneralMatrix();
        Matrix4f inPlace = matrix;
        inPlace.Transpose();

        const Matrix4f transposed = matrix.GetTransposed();

        for (std::size_t row = 0; row < 4; ++row) {
            for (std::size_t col = 0; col < 4; ++col) {
                EXPECT_FLOAT_EQ(transposed(row, col), matrix(col, row));
            }
        }
        EXPECT_TRUE(MatricesNear(inPlace, transposed));
    }

    TEST(MatrixTests, Matrix3TransposeSwapsRowsAndColumns) {
        const Matrix3f matrix = General3x3();
        Matrix3f inPlace = matrix;
        inPlace.Transpose();

        const Matrix3f transposed = matrix.GetTransposed();

        for (std::size_t row = 0; row < 3; ++row) {
            for (std::size_t col = 0; col < 3; ++col) {
                EXPECT_FLOAT_EQ(transposed(row, col), matrix(col, row));
                EXPECT_FLOAT_EQ(inPlace(row, col), matrix(col, row));
            }
        }
    }

    TEST(MatrixTests, TransformVectorIgnoresTranslationButTransformPointDoesNot) {
        const Matrix4f transform = MatrixFactory::CreateTranslation(Vector3f{5.0f, 0.0f, 0.0f}) * MatrixFactory::CreateScale(2.0f);
        const Vector3f v{1.0f, 2.0f, 3.0f};

        EXPECT_TRUE(VectorsNear(transform.TransformVector(v), Vector3f{2.0f, 4.0f, 6.0f}));
        EXPECT_TRUE(VectorsNear(transform.TransformPoint(v), Vector3f{7.0f, 4.0f, 6.0f}));
    }

    TEST(MatrixTests, TransformPointDividesByW) {
        Matrix4f projective;
        projective(3, 3) = 2.0f;

        EXPECT_TRUE(VectorsNear(projective.TransformPoint(Vector3f{2.0f, 4.0f, 6.0f}), Vector3f{1.0f, 2.0f, 3.0f}));
    }

    TEST(MatrixTests, MultiplyingAVector4MatchesTransformPointForW1) {
        const Matrix4f transform = MatrixFactory::CreateTranslation(Vector3f{1.0f, 2.0f, 3.0f}) * MatrixFactory::CreateRotationX(0.4f);
        const Vector3f point{-1.0f, 0.5f, 2.0f};

        const Vector4f result = transform * Vector4f{point, 1.0f};

        EXPECT_TRUE(VectorsNear(Vector3f{result.x, result.y, result.z}, transform.TransformPoint(point)));
        EXPECT_FLOAT_EQ(result.w, 1.0f);
    }

    TEST(MatrixTests, Matrix3FromMatrix4KeepsTheUpperLeftBlock) {
        const Matrix4f matrix = GeneralMatrix();

        const Matrix3f block{matrix};

        for (std::size_t row = 0; row < 3; ++row) {
            for (std::size_t col = 0; col < 3; ++col) {
                EXPECT_FLOAT_EQ(block(row, col), matrix(row, col));
            }
        }
    }

    TEST(MatrixTests, Matrix3TransformVectorMatchesTheMatrix4Rotation) {
        const Matrix4f rotation = MatrixFactory::CreateRotationY(1.1f);
        const Vector3f v{0.3f, -2.0f, 1.0f};

        EXPECT_TRUE(VectorsNear(Matrix3f{rotation}.TransformVector(v), rotation.TransformVector(v)));
    }

    TEST(MatrixTests, OperationsWorkAtCompileTime) {
        constexpr Matrix4f product = Matrix4f::Identity() * MatrixFactory::CreateTranslation(Vector3f{1.0f, 2.0f, 3.0f});
        static_assert(product(0, 3) == 1.0f);
        static_assert(product.GetTransposed()(3, 2) == 3.0f);
        SUCCEED();
    }
}
