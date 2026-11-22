// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Physics {
    struct Capsule;
    struct OrientedBox;
    struct Plane;
    struct Sphere;
}

namespace SpinoCore::Core::Actors::Components {
    class ColliderComponent;
}

namespace SpinoCore::Core::Rendering {
    struct DebugDrawList;

    void AddDebugLine(DebugDrawList& list, const Math::Vector3f& from, const Math::Vector3f& to, const Math::Vector4f& color);
    void AddDebugBox(DebugDrawList& list, const Physics::OrientedBox& box, const Math::Vector4f& color);
    void AddDebugSphere(DebugDrawList& list, const Physics::Sphere& sphere, const Math::Vector4f& color);
    void AddDebugCapsule(DebugDrawList& list, const Physics::Capsule& capsule, const Math::Vector4f& color);
    void AddDebugPlane(DebugDrawList& list, const Physics::Plane& plane, const Math::Vector3f& around, const Math::Vector4f& color);

    [[nodiscard]] Math::Vector4f DebugColorOf(const Actors::Components::ColliderComponent& collider);
}
