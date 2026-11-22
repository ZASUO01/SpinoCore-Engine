// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <functional>
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::Actors::Components {
    class ColliderComponent;
}

namespace SpinoCore::Physics {
    struct Ray final {
        Math::Vector3f origin{Math::Vector3f::Zero()};
        Math::Vector3f direction{Math::Vector3f::NegUnitZ()};

        [[nodiscard]] Math::Vector3f PointAt(const float distance) const { return origin + direction * distance; }
    };

    struct RaycastHit final {
        Core::Actors::Components::ColliderComponent* collider{nullptr};
        Math::Vector3f point{Math::Vector3f::Zero()};
        Math::Vector3f normal{Math::Vector3f::UnitY()};
        float distance{0.0f};
    };

    using RaycastFilter = std::function<bool(const Core::Actors::Components::ColliderComponent&)>;
}
