// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <array>
#include <optional>
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Physics {
    struct OrientedBox final {
        Math::Vector3f center{Math::Vector3f::Zero()};
        std::array<Math::Vector3f, 3> axes{Math::Vector3f::UnitX(), Math::Vector3f::UnitY(), Math::Vector3f::UnitZ()};
        Math::Vector3f halfExtents{Math::Vector3f::One()};
    };

    struct Contact final {
        Math::Vector3f normal{Math::Vector3f::UnitX()};
        float depth{0.0f};
    };

    [[nodiscard]] OrientedBox MakeOrientedBox(const Math::Matrix4f& world, const Math::Vector3f& halfExtents);
    [[nodiscard]] std::optional<Contact> TestOrientedBoxes(const OrientedBox& a, const OrientedBox& b);
}
