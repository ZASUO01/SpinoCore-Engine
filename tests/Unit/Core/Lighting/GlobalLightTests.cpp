// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <cmath>
#include <gtest/gtest.h>
#include "SpinoCore/Core/Lighting/GlobalLight.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Core::Lighting::GlobalLight;
    using SpinoCore::Core::Lighting::GlobalLightSettings;
    using SpinoCore::Math::Vector3f;

    [[nodiscard]] GlobalLight LightAbove(const Vector3f& target = Vector3f::Zero()) {
        return GlobalLight{true, GlobalLightSettings{
            .position = target + Vector3f{0.0f, 10.0f, 0.0f},
            .target = target,
            .color = Vector3f::One(),
            .intensity = 1.0f,
            .ambientColor = Vector3f::Zero(),
            .shadowExtent = 5.0f,
            .shadowNear = 0.1f,
            .shadowFar = 20.0f,
        }};
    }

    [[nodiscard]] Vector3f ProjectedByTheLight(const GlobalLight& light, const Vector3f& point) {
        return light.GetViewProjection().TransformPoint(point);
    }

    TEST(GlobalLightTests, IsDisabledByDefault) {
        const GlobalLight light;

        EXPECT_FALSE(light.IsEnabled());
    }

    TEST(GlobalLightTests, PointsFromItsPositionToItsTarget) {
        GlobalLight light{true};
        light.SetPosition(Vector3f{0.0f, 4.0f, 3.0f});
        light.SetTarget(Vector3f::Zero());

        EXPECT_TRUE(Vector3f::NearlyEqual(light.GetDirection(), Vector3f{0.0f, -0.8f, -0.6f}));
    }

    TEST(GlobalLightTests, ChangingTheHeightTiltsTheDirection) {
        GlobalLight light{true};
        light.SetPosition(Vector3f{3.0f, 1.0f, 0.0f});
        light.SetTarget(Vector3f::Zero());

        light.SetHeight(3.0f);

        EXPECT_TRUE(Vector3f::NearlyEqual(light.GetDirection(), Vector3f{-1.0f, -1.0f, 0.0f}.Normalized()));
    }

    TEST(GlobalLightTests, TheTargetIsProjectedToTheCenterOfTheShadowMap) {
        const GlobalLight light = LightAbove(Vector3f{7.0f, 0.0f, -2.0f});

        const Vector3f projected = ProjectedByTheLight(light, Vector3f{7.0f, 0.0f, -2.0f});

        EXPECT_NEAR(projected.x, 0.0f, 1e-5f);
        EXPECT_NEAR(projected.y, 0.0f, 1e-5f);
    }

    TEST(GlobalLightTests, PointsAtTheShadowExtentReachTheEdgeOfTheShadowMap) {
        const GlobalLight light = LightAbove();

        const Vector3f projected = ProjectedByTheLight(light, Vector3f{5.0f, 0.0f, 0.0f});

        EXPECT_NEAR(std::abs(projected.x), 1.0f, 1e-5f);
    }

    TEST(GlobalLightTests, ALargerExtentFitsMoreOfTheSceneInTheShadowMap) {
        GlobalLight light = LightAbove();

        light.SetShadowExtent(10.0f);

        EXPECT_NEAR(std::abs(ProjectedByTheLight(light, Vector3f{5.0f, 0.0f, 0.0f}).x), 0.5f, 1e-5f);
    }

    TEST(GlobalLightTests, PointsCloserToTheLightHaveSmallerDepth) {
        const GlobalLight light = LightAbove();

        const float nearDepth = ProjectedByTheLight(light, Vector3f{0.0f, 5.0f, 0.0f}).z;
        const float farDepth = ProjectedByTheLight(light, Vector3f::Zero()).z;

        EXPECT_LT(nearDepth, farDepth);
    }

    TEST(GlobalLightTests, ALightStraightAboveItsTargetStillHasValidMatrices) {
        const GlobalLight light = LightAbove();

        const Vector3f projected = ProjectedByTheLight(light, Vector3f::Zero());

        EXPECT_FALSE(std::isnan(projected.x));
    }
}
