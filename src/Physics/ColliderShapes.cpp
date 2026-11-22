// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "ColliderShapes.h"
#include <algorithm>
#include <initializer_list>
#include "SpinoCore/Core/Actors/Actor.h"
#include "SpinoCore/Core/Actors/Components/BoxColliderComponent.h"
#include "SpinoCore/Core/Actors/Components/CapsuleColliderComponent.h"
#include "SpinoCore/Core/Actors/Components/PlaneColliderComponent.h"
#include "SpinoCore/Core/Actors/Components/SphereColliderComponent.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Physics {
    namespace {
        [[nodiscard]] const Math::Matrix4f& WorldOf(const Core::Actors::Components::ColliderComponent& collider) {
            return collider.GetActor()->GetWorldTransform();
        }

        [[nodiscard]] OrientedBox FrameOf(const Core::Actors::Components::ColliderComponent& collider) {
            return MakeOrientedBox(WorldOf(collider), Math::Vector3f::One());
        }
    }

    OrientedBox BoxOf(const Core::Actors::Components::BoxColliderComponent& collider) {
        return MakeOrientedBox(WorldOf(collider), collider.GetExtent());
    }

    Sphere SphereOf(const Core::Actors::Components::SphereColliderComponent& collider) {
        const Math::Vector3f scale = FrameOf(collider).halfExtents;
        return Sphere{
            .center = WorldOf(collider).TransformPoint(collider.GetOffset()),
            .radius = collider.GetRadius() * std::max({scale.x, scale.y, scale.z}),
        };
    }

    Capsule CapsuleOf(const Core::Actors::Components::CapsuleColliderComponent& collider) {
        const Math::Vector3f scale = FrameOf(collider).halfExtents;
        const Math::Matrix4f& world = WorldOf(collider);
        return Capsule{
            .start = world.TransformPoint(Math::Vector3f{0.0f, -collider.GetHalfHeight(), 0.0f}),
            .end = world.TransformPoint(Math::Vector3f{0.0f, collider.GetHalfHeight(), 0.0f}),
            .radius = collider.GetRadius() * std::max(scale.x, scale.z),
        };
    }

    Plane PlaneOf(const Core::Actors::Components::PlaneColliderComponent& collider) {
        const OrientedBox frame = FrameOf(collider);
        const Math::Vector3f& local = collider.GetNormal();
        Math::Vector3f normal = frame.axes[0] * local.x + frame.axes[1] * local.y + frame.axes[2] * local.z;
        normal = normal.LengthSq() > 0.0f ? normal.Normalized() : Math::Vector3f::UnitY();

        const Math::Vector3f point = frame.center + normal * collider.GetDistanceToOrigin();
        return Plane{.normal = normal, .distance = normal.Dot(point)};
    }
}
