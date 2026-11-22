// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "OrientedBox.h"
#include "Shapes.h"

namespace SpinoCore::Core::Actors::Components {
    class BoxColliderComponent;
    class CapsuleColliderComponent;
    class PlaneColliderComponent;
    class SphereColliderComponent;
}

namespace SpinoCore::Physics {
    [[nodiscard]] OrientedBox BoxOf(const Core::Actors::Components::BoxColliderComponent& collider);
    [[nodiscard]] Sphere SphereOf(const Core::Actors::Components::SphereColliderComponent& collider);
    [[nodiscard]] Capsule CapsuleOf(const Core::Actors::Components::CapsuleColliderComponent& collider);
    [[nodiscard]] Plane PlaneOf(const Core::Actors::Components::PlaneColliderComponent& collider);
}
