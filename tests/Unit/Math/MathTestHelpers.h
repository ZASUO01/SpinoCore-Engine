// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstddef>
#include <gtest/gtest.h>
#include "SpinoCore/Math/Math.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Quaternion.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Tests {
    inline constexpr float MathTolerance = 1e-5f;

    inline testing::AssertionResult VectorsNear(const Math::Vector3f& actual, const Math::Vector3f& expected,
                                                const float tolerance = MathTolerance) {
        if (Math::Vector3f::NearlyEqual(actual, expected, tolerance)) return testing::AssertionSuccess();
        return testing::AssertionFailure() << "got (" << actual.x << ", " << actual.y << ", " << actual.z
                                           << "), expected (" << expected.x << ", " << expected.y << ", " << expected.z << ")";
    }

    inline testing::AssertionResult MatricesNear(const Math::Matrix4f& actual, const Math::Matrix4f& expected,
                                                 const float tolerance = MathTolerance) {
        for (std::size_t row = 0; row < 4; ++row) {
            for (std::size_t col = 0; col < 4; ++col) {
                if (!Math::Operations::NearlyEqual(actual(row, col), expected(row, col), tolerance)) {
                    return testing::AssertionFailure() << "element (" << row << ", " << col << ") is " << actual(row, col)
                                                       << ", expected " << expected(row, col);
                }
            }
        }
        return testing::AssertionSuccess();
    }

    inline testing::AssertionResult QuaternionsNear(const Math::Quaternionf& actual, const Math::Quaternionf& expected,
                                                    const float tolerance = MathTolerance) {
        if (Math::Quaternionf::NearlyEqual(actual, expected, tolerance)) return testing::AssertionSuccess();
        return testing::AssertionFailure() << "got (" << actual.x << ", " << actual.y << ", " << actual.z << ", " << actual.w
                                           << "), expected (" << expected.x << ", " << expected.y << ", " << expected.z
                                           << ", " << expected.w << ")";
    }
}
