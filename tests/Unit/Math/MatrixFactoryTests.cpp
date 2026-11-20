// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <cmath>
#include <gtest/gtest.h>
#include "MathTestHelpers.h"
#include "SpinoCore/Math/Math.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/MatrixFactory.h"
#include "SpinoCore/Math/Quaternion.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Math::Matrix4f;
    using SpinoCore::Math::Quaternionf;
    using SpinoCore::Math::Vector3f;
    using SpinoCore::Math::Vector4f;
    using SpinoCore::Tests::MatricesNear;
    using SpinoCore::Tests::VectorsNear;
    namespace MatrixFactory = SpinoCore::Math::MatrixFactory;
    namespace Operations = SpinoCore::Math::Operations;

    constexpr float HalfPi = SpinoCore::Math::Constants::PIOver2<float>;

    // Projects a point and returns its normalized device coordinates.
    [[nodiscard]] Vector3f ToNDC(const Matrix4f& projection, const Vector3f& point) {
        const Vector4f clip = projection * Vector4f{point, 1.0f};
        return Vector3f{clip.x / clip.w, clip.y / clip.w, clip.z / clip.w};
    }

    [[nodiscard]] bool IsFinite(const Matrix4f& matrix) {
        for (const float value : matrix.data) {
            if (!std::isfinite(value)) return false;
        }
        return true;
    }

    TEST(MatrixFactoryTests, IdentityAxesFollowTheOpenGLConvention) {
        const Matrix4f identity;

        EXPECT_TRUE(VectorsNear(MatrixFactory::GetRight(identity), Vector3f::UnitX()));
        EXPECT_TRUE(VectorsNear(MatrixFactory::GetUp(identity), Vector3f::UnitY()));
        EXPECT_TRUE(VectorsNear(MatrixFactory::GetForward(identity), Vector3f::NegUnitZ()));
    }

    TEST(MatrixFactoryTests, TranslationMovesPointsButNotDirections) {
        const Matrix4f translation = MatrixFactory::CreateTranslation(Vector3f{1.0f, 2.0f, 3.0f});

        EXPECT_TRUE(VectorsNear(translation.TransformPoint(Vector3f::Zero()), Vector3f{1.0f, 2.0f, 3.0f}));
        EXPECT_TRUE(VectorsNear(translation.TransformVector(Vector3f::UnitX()), Vector3f::UnitX()));
        EXPECT_TRUE(VectorsNear(MatrixFactory::GetTranslation(translation), Vector3f{1.0f, 2.0f, 3.0f}));
    }

    TEST(MatrixFactoryTests, ScaleOverloadsAgree) {
        EXPECT_TRUE(MatricesNear(MatrixFactory::CreateScale(2.0f), MatrixFactory::CreateScale(2.0f, 2.0f, 2.0f)));
        EXPECT_TRUE(MatricesNear(MatrixFactory::CreateScale(Vector3f{1.0f, 2.0f, 3.0f}), MatrixFactory::CreateScale(1.0f, 2.0f, 3.0f)));
        EXPECT_TRUE(VectorsNear(MatrixFactory::CreateScale(1.0f, 2.0f, 3.0f).TransformPoint(Vector3f::One()), Vector3f{1.0f, 2.0f, 3.0f}));
    }

    // Positive angles turn counter-clockwise when looking down the axis toward the origin (right-handed).
    TEST(MatrixFactoryTests, RotationsAreCounterClockwiseAndRightHanded) {
        EXPECT_TRUE(VectorsNear(MatrixFactory::CreateRotationX(HalfPi).TransformVector(Vector3f::UnitY()), Vector3f::UnitZ()));
        EXPECT_TRUE(VectorsNear(MatrixFactory::CreateRotationY(HalfPi).TransformVector(Vector3f::UnitZ()), Vector3f::UnitX()));
        EXPECT_TRUE(VectorsNear(MatrixFactory::CreateRotationZ(HalfPi).TransformVector(Vector3f::UnitX()), Vector3f::UnitY()));
    }

    TEST(MatrixFactoryTests, QuaternionMatricesMatchTheAxisRotations) {
        const float angle = 0.83f;

        EXPECT_TRUE(MatricesNear(MatrixFactory::CreateFromQuaternion(Quaternionf{Vector3f::UnitX(), angle}), MatrixFactory::CreateRotationX(angle)));
        EXPECT_TRUE(MatricesNear(MatrixFactory::CreateFromQuaternion(Quaternionf{Vector3f::UnitY(), angle}), MatrixFactory::CreateRotationY(angle)));
        EXPECT_TRUE(MatricesNear(MatrixFactory::CreateFromQuaternion(Quaternionf{Vector3f::UnitZ(), angle}), MatrixFactory::CreateRotationZ(angle)));
    }

    TEST(MatrixFactoryTests, QuaternionMatrixRotatesLikeTheQuaternion) {
        const Quaternionf rotation{Vector3f{1.0f, 2.0f, -1.0f}.Normalized(), 1.3f};
        const Vector3f v{0.4f, -1.0f, 2.5f};

        EXPECT_TRUE(VectorsNear(MatrixFactory::CreateFromQuaternion(rotation).TransformVector(v), rotation.Rotate(v)));
    }

    TEST(MatrixFactoryTests, TransformScalesThenRotatesThenTranslates) {
        const Vector3f translation{10.0f, 0.0f, 0.0f};
        const Quaternionf rotation{Vector3f::UnitZ(), HalfPi};
        const Vector3f scale{2.0f, 1.0f, 1.0f};

        const Matrix4f transform = MatrixFactory::CreateTransform(translation, rotation, scale);

        // (1, 0, 0) -> scaled (2, 0, 0) -> rotated (0, 2, 0) -> translated (10, 2, 0).
        EXPECT_TRUE(VectorsNear(transform.TransformPoint(Vector3f::UnitX()), Vector3f{10.0f, 2.0f, 0.0f}));
    }

    TEST(MatrixFactoryTests, ScaleAndTranslationCanBeReadBackFromATransform) {
        const Matrix4f transform = MatrixFactory::CreateTransform(
            Vector3f{1.0f, 2.0f, 3.0f}, Quaternionf::FromEuler(0.3f, -0.6f, 1.1f), Vector3f{2.0f, 3.0f, 4.0f});

        EXPECT_TRUE(VectorsNear(MatrixFactory::GetScale(transform), Vector3f{2.0f, 3.0f, 4.0f}));
        EXPECT_TRUE(VectorsNear(MatrixFactory::GetTranslation(transform), Vector3f{1.0f, 2.0f, 3.0f}));
    }

    // Euler angles are (pitch about X, yaw about Y, roll about Z), applied as yaw * pitch * roll.
    TEST(MatrixFactoryTests, EulerAnglesComposeAsYawPitchRoll) {
        const float pitch = 0.4f;
        const float yaw = -1.2f;
        const float roll = 0.7f;

        const Matrix4f expected = MatrixFactory::CreateRotationY(yaw) * MatrixFactory::CreateRotationX(pitch) *
                                  MatrixFactory::CreateRotationZ(roll);

        EXPECT_TRUE(MatricesNear(MatrixFactory::CreateFromQuaternion(Quaternionf::FromEuler(pitch, yaw, roll)), expected));
    }

    TEST(MatrixFactoryTests, EulerAnglesRoundTripThroughAMatrix) {
        const Vector3f angles{0.4f, -1.2f, 0.7f};
        const Matrix4f rotation = MatrixFactory::CreateFromQuaternion(Quaternionf::FromEuler(angles.x, angles.y, angles.z));

        EXPECT_TRUE(VectorsNear(MatrixFactory::GetRotationEuler(rotation), angles, 1e-4f));
    }

    TEST(MatrixFactoryTests, EulerAnglesIgnoreScale) {
        const Vector3f angles{-0.3f, 2.0f, -0.5f};
        const Matrix4f transform = MatrixFactory::CreateTransform(
            Vector3f::Zero(), Quaternionf::FromEuler(angles.x, angles.y, angles.z), Vector3f{3.0f, 0.5f, 2.0f});

        EXPECT_TRUE(VectorsNear(MatrixFactory::GetRotationEuler(transform), angles, 1e-4f));
    }

    // Looking straight up or down loses one degree of freedom; the angles must still rebuild the same rotation.
    TEST(MatrixFactoryTests, EulerAnglesAtGimbalLockStillDescribeTheSameRotation) {
        const Matrix4f rotation = MatrixFactory::CreateFromQuaternion(Quaternionf::FromEuler(HalfPi, 0.5f, 0.3f));

        const Vector3f angles = MatrixFactory::GetRotationEuler(rotation);
        const Matrix4f rebuilt = MatrixFactory::CreateFromQuaternion(Quaternionf::FromEuler(angles.x, angles.y, angles.z));

        EXPECT_TRUE(MatricesNear(rebuilt, rotation, 1e-3f));
    }

    TEST(MatrixFactoryTests, LookAtPutsTheEyeAtTheOriginLookingDownNegativeZ) {
        const Vector3f eye{3.0f, 2.0f, 5.0f};
        const Vector3f target{-1.0f, 0.0f, 1.0f};

        const Matrix4f view = MatrixFactory::CreateLookAt(eye, target, Vector3f::UnitY());

        EXPECT_TRUE(VectorsNear(view.TransformPoint(eye), Vector3f::Zero()));
        const Vector3f targetInView = view.TransformPoint(target);
        EXPECT_TRUE(VectorsNear(targetInView, Vector3f{0.0f, 0.0f, -(target - eye).Length()}, 1e-4f));
    }

    TEST(MatrixFactoryTests, LookAtKeepsTheWorldUpPointingUp) {
        const Matrix4f view = MatrixFactory::CreateLookAt(Vector3f{0.0f, 0.0f, 5.0f}, Vector3f::Zero(), Vector3f::UnitY());

        EXPECT_TRUE(VectorsNear(view.TransformVector(Vector3f::UnitY()), Vector3f::UnitY()));
        EXPECT_TRUE(VectorsNear(view.TransformVector(Vector3f::UnitX()), Vector3f::UnitX()));
    }

    TEST(MatrixFactoryTests, LookAtAlongTheUpVectorStillGivesAnOrthonormalView) {
        const Matrix4f view = MatrixFactory::CreateLookAt(Vector3f::Zero(), Vector3f{0.0f, 10.0f, 0.0f}, Vector3f::UnitY());

        ASSERT_TRUE(IsFinite(view));
        EXPECT_NEAR(MatrixFactory::GetRight(view).Length(), 1.0f, 1e-5f);
        EXPECT_NEAR(MatrixFactory::GetUp(view).Length(), 1.0f, 1e-5f);
        EXPECT_TRUE(VectorsNear(view.TransformPoint(Vector3f{0.0f, 10.0f, 0.0f}), Vector3f{0.0f, 0.0f, -10.0f}, 1e-4f));
    }

    TEST(MatrixFactoryTests, LookAtTheEyeItselfGivesIdentity) {
        EXPECT_TRUE(MatricesNear(MatrixFactory::CreateLookAt(Vector3f::One(), Vector3f::One(), Vector3f::UnitY()), Matrix4f::Identity()));
    }

    TEST(MatrixFactoryTests, OrthoMapsTheBoxOntoNormalizedDeviceCoordinates) {
        const Matrix4f ortho = MatrixFactory::CreateOrtho(0.0f, 800.0f, 0.0f, 600.0f, 0.1f, 100.0f);

        EXPECT_TRUE(VectorsNear(ToNDC(ortho, Vector3f{0.0f, 0.0f, -0.1f}), Vector3f{-1.0f, -1.0f, -1.0f}));
        EXPECT_TRUE(VectorsNear(ToNDC(ortho, Vector3f{800.0f, 600.0f, -100.0f}), Vector3f{1.0f, 1.0f, 1.0f}));
        EXPECT_TRUE(VectorsNear(ToNDC(ortho, Vector3f{400.0f, 300.0f, -0.1f}), Vector3f{0.0f, 0.0f, -1.0f}));
    }

    TEST(MatrixFactoryTests, CenteredOrthoMatchesTheBoxOrtho) {
        EXPECT_TRUE(MatricesNear(MatrixFactory::CreateOrtho(8.0f, 6.0f, 0.5f, 50.0f),
                                 MatrixFactory::CreateOrtho(-4.0f, 4.0f, -3.0f, 3.0f, 0.5f, 50.0f)));
    }

    TEST(MatrixFactoryTests, DegenerateOrthoGivesIdentityInsteadOfInfinities) {
        EXPECT_TRUE(MatricesNear(MatrixFactory::CreateOrtho(0.0f, 0.0f, 0.0f, 1.0f, 0.1f, 1.0f), Matrix4f::Identity()));
        EXPECT_TRUE(MatricesNear(MatrixFactory::CreateOrtho(0.0f, 6.0f, 0.1f, 1.0f), Matrix4f::Identity()));
        EXPECT_TRUE(MatricesNear(MatrixFactory::CreateOrtho(8.0f, 6.0f, 1.0f, 1.0f), Matrix4f::Identity()));
    }

    TEST(MatrixFactoryTests, PerspectiveMapsTheNearAndFarPlanesToTheDepthRange) {
        const Matrix4f projection = MatrixFactory::CreatePerspectiveFOV(HalfPi, 800.0f, 600.0f, 0.1f, 100.0f);

        EXPECT_NEAR(ToNDC(projection, Vector3f{0.0f, 0.0f, -0.1f}).z, -1.0f, 1e-4f);
        EXPECT_NEAR(ToNDC(projection, Vector3f{0.0f, 0.0f, -100.0f}).z, 1.0f, 1e-4f);
    }

    TEST(MatrixFactoryTests, PerspectiveUsesAVerticalFieldOfViewAndTheAspectRatio) {
        const Matrix4f projection = MatrixFactory::CreatePerspectiveFOV(HalfPi, 800.0f, 600.0f, 0.1f, 100.0f);

        // With a 90 degree vertical FOV the top edge at depth d is at y = d.
        EXPECT_NEAR(ToNDC(projection, Vector3f{0.0f, 5.0f, -5.0f}).y, 1.0f, 1e-5f);
        // The right edge is wider by the aspect ratio.
        EXPECT_NEAR(ToNDC(projection, Vector3f{5.0f * 800.0f / 600.0f, 0.0f, -5.0f}).x, 1.0f, 1e-5f);
    }

    TEST(MatrixFactoryTests, PerspectiveWithAnEmptyViewportGivesIdentity) {
        EXPECT_TRUE(MatricesNear(MatrixFactory::CreatePerspectiveFOV(HalfPi, 0.0f, 600.0f, 0.1f, 100.0f), Matrix4f::Identity()));
    }

    TEST(MatrixFactoryTests, RotationMatricesAreOrthonormal) {
        const Matrix4f rotation = MatrixFactory::CreateFromQuaternion(Quaternionf::FromEuler(0.2f, 1.4f, -0.9f));

        EXPECT_TRUE(MatricesNear(rotation * rotation.GetTransposed(), Matrix4f::Identity()));
        EXPECT_NEAR(Operations::Abs(MatrixFactory::GetRight(rotation).Dot(MatrixFactory::GetUp(rotation))), 0.0f, 1e-5f);
    }
}
