// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <gtest/gtest.h>
#include "Core/Cameras/DefaultCamera.h"
#include "SpinoCore/Core/Cameras/Camera.h"
#include "SpinoCore/Math/Math.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Core::Cameras::Camera;
    using SpinoCore::Core::Cameras::DefaultCamera;
    using SpinoCore::Math::Matrix4f;
    using SpinoCore::Math::Vector3f;
    using SpinoCore::Math::Vector4f;
    namespace Operations = SpinoCore::Math::Operations;

    constexpr float Tolerance = 1e-4f;

    [[nodiscard]] Vector3f ToNDC(const Camera& camera, const Vector3f& worldPoint) {
        const Vector4f clip = camera.GetProjectionMatrix() * (camera.GetViewMatrix() * Vector4f{worldPoint, 1.0f});
        return Vector3f{clip.x / clip.w, clip.y / clip.w, clip.z / clip.w};
    }

    [[nodiscard]] bool IsIdentity(const Matrix4f& matrix) {
        for (std::size_t row = 0; row < 4; ++row) {
            for (std::size_t col = 0; col < 4; ++col) {
                if (matrix(row, col) != (row == col ? 1.0f : 0.0f)) return false;
            }
        }
        return true;
    }

    [[nodiscard]] bool IsFinite(const Matrix4f& matrix) {
        return std::ranges::all_of(matrix.data, [](const float value) { return std::isfinite(value); });
    }

    class BareCamera final : public Camera {
    public:
        void Update(float) override {}
    };

    constexpr float DefaultEyeDistance = 5.196152f;
    const float HalfFovTangent = Operations::Tan(Operations::ToRadians(45.0f) / 2.0f);

    TEST(CameraTests, ABareCameraStartsWithIdentityMatrices) {
        const BareCamera camera;

        EXPECT_TRUE(IsIdentity(camera.GetViewMatrix()));
        EXPECT_TRUE(IsIdentity(camera.GetProjectionMatrix()));
    }

    TEST(CameraTests, ABareCameraWithoutPlanesKeepsAFiniteProjectionAfterAResize) {
        BareCamera camera;

        camera.SetViewportSize(800.0f, 600.0f);

        EXPECT_TRUE(IsFinite(camera.GetProjectionMatrix()));
    }

    TEST(DefaultCameraTests, LooksAtTheOriginFromAboveAndToTheSide) {
        const DefaultCamera camera{800.0f, 600.0f};

        const Vector3f eyeInView = camera.GetViewMatrix().TransformPoint(Vector3f{3.0f, 3.0f, 3.0f});
        const Vector3f originInView = camera.GetViewMatrix().TransformPoint(Vector3f::Zero());

        EXPECT_TRUE(Vector3f::NearlyEqual(eyeInView, Vector3f::Zero(), Tolerance));
        EXPECT_TRUE(Vector3f::NearlyEqual(originInView, Vector3f{0.0f, 0.0f, -DefaultEyeDistance}, Tolerance));
    }

    TEST(DefaultCameraTests, TheOriginLandsInTheCenterOfTheScreen) {
        const DefaultCamera camera{800.0f, 600.0f};

        const Vector3f ndc = ToNDC(camera, Vector3f::Zero());

        EXPECT_NEAR(ndc.x, 0.0f, Tolerance);
        EXPECT_NEAR(ndc.y, 0.0f, Tolerance);
        EXPECT_GT(ndc.z, -1.0f);
        EXPECT_LT(ndc.z, 1.0f);
    }

    TEST(DefaultCameraTests, UsesA45DegreeVerticalFieldOfView) {
        const DefaultCamera camera{800.0f, 600.0f};
        const float depth = 10.0f;

        const Vector4f clip = camera.GetProjectionMatrix() * Vector4f{0.0f, HalfFovTangent * depth, -depth, 1.0f};

        EXPECT_NEAR(clip.y / clip.w, 1.0f, Tolerance);
    }

    TEST(DefaultCameraTests, WidensTheViewByTheAspectRatio) {
        const DefaultCamera camera{800.0f, 600.0f};
        const float depth = 10.0f;
        const float aspect = 800.0f / 600.0f;

        const Vector4f clip = camera.GetProjectionMatrix() * Vector4f{HalfFovTangent * depth * aspect, 0.0f, -depth, 1.0f};

        EXPECT_NEAR(clip.x / clip.w, 1.0f, Tolerance);
    }

    TEST(DefaultCameraTests, MapsTheNearAndFarPlanesToTheDepthRange) {
        const DefaultCamera camera{800.0f, 600.0f};

        const Vector4f nearClip = camera.GetProjectionMatrix() * Vector4f{0.0f, 0.0f, -0.1f, 1.0f};
        const Vector4f farClip = camera.GetProjectionMatrix() * Vector4f{0.0f, 0.0f, -1000.0f, 1.0f};

        EXPECT_NEAR(nearClip.z / nearClip.w, -1.0f, Tolerance);
        EXPECT_NEAR(farClip.z / farClip.w, 1.0f, 1e-3f);
    }

    TEST(DefaultCameraTests, ResizingTheViewportChangesTheAspectRatioButNotTheView) {
        DefaultCamera camera{800.0f, 600.0f};
        const Matrix4f viewBefore = camera.GetViewMatrix();
        const float depth = 10.0f;

        camera.SetViewportSize(500.0f, 500.0f);

        const Vector4f clip = camera.GetProjectionMatrix() * Vector4f{HalfFovTangent * depth, 0.0f, -depth, 1.0f};
        EXPECT_NEAR(clip.x / clip.w, 1.0f, Tolerance);
        for (std::size_t i = 0; i < 16; ++i) EXPECT_FLOAT_EQ(camera.GetViewMatrix().data[i], viewBefore.data[i]);
    }

    TEST(DefaultCameraTests, AnEmptyViewportGivesAFiniteProjection) {
        const DefaultCamera camera{0.0f, 0.0f};

        EXPECT_TRUE(IsFinite(camera.GetProjectionMatrix()));
    }
}
