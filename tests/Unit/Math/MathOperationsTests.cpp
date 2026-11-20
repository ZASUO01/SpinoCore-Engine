// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <cmath>
#include <gtest/gtest.h>
#include "SpinoCore/Math/Math.h"

namespace {
    namespace Constants = SpinoCore::Math::Constants;
    namespace Operations = SpinoCore::Math::Operations;

    TEST(MathOperationsTests, NearlyEqualAcceptsDifferencesWithinTheTolerance) {
        EXPECT_TRUE(Operations::NearlyEqual(1.0f, 1.000001f));
        EXPECT_FALSE(Operations::NearlyEqual(1.0f, 1.001f));
    }

    TEST(MathOperationsTests, NearlyEqualScalesTheToleranceWithLargeValues) {
        EXPECT_TRUE(Operations::NearlyEqual(1'000'000.0f, 1'000'001.0f));
        EXPECT_FALSE(Operations::NearlyEqual(1'000'000.0f, 1'000'100.0f));
    }

    TEST(MathOperationsTests, NearlyZeroUsesAnAbsoluteTolerance) {
        EXPECT_TRUE(Operations::NearlyZero(-1e-6f));
        EXPECT_FALSE(Operations::NearlyZero(1e-3f));
    }

    TEST(MathOperationsTests, ConvertsBetweenDegreesAndRadians) {
        EXPECT_FLOAT_EQ(Operations::ToRadians(180.0f), Constants::PI<float>);
        EXPECT_FLOAT_EQ(Operations::ToDegrees(Constants::PIOver2<float>), 90.0f);
        EXPECT_DOUBLE_EQ(Operations::ToDegrees(Operations::ToRadians(37.5)), 37.5);
    }

    TEST(MathOperationsTests, AsinAndAcosClampInputsThatDriftOutsideTheirDomain) {
        const float slightlyAboveOne = 1.0f + 1e-6f;

        EXPECT_FALSE(std::isnan(Operations::Asin(slightlyAboveOne)));
        EXPECT_FLOAT_EQ(Operations::Asin(slightlyAboveOne), Constants::PIOver2<float>);
        EXPECT_FLOAT_EQ(Operations::Acos(-slightlyAboveOne), Constants::PI<float>);
    }

    TEST(MathOperationsTests, SgnReturnsTheSignAsMinusOneZeroOrOne) {
        EXPECT_EQ(Operations::Sgn(-5), -1);
        EXPECT_EQ(Operations::Sgn(0), 0);
        EXPECT_EQ(Operations::Sgn(3), 1);
        EXPECT_FLOAT_EQ(Operations::Sgn(-0.25f), -1.0f);
    }

    TEST(MathOperationsTests, AbsWorksForSignedAndUnsignedTypes) {
        EXPECT_EQ(Operations::Abs(-4), 4);
        EXPECT_EQ(Operations::Abs(4u), 4u);
        EXPECT_FLOAT_EQ(Operations::Abs(-2.5f), 2.5f);
    }

    TEST(MathOperationsTests, AngleConstantsAreConsistent) {
        EXPECT_FLOAT_EQ(Constants::TwoPI<float>, 2.0f * Constants::PI<float>);
        EXPECT_FLOAT_EQ(Constants::PIOver4<float> * 4.0f, Constants::PI<float>);
        EXPECT_FLOAT_EQ(Constants::DegToRad<float> * Constants::RadToDeg<float>, 1.0f);
    }
}
