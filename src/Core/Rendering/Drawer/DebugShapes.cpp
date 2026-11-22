// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "DebugShapes.h"
#include <array>
#include <cstddef>
#include <initializer_list>
#include "DrawCommands.h"
#include "Physics/OrientedBox.h"
#include "Physics/Shapes.h"
#include "SpinoCore/Core/Actors/Components/ColliderComponent.h"
#include "SpinoCore/Math/Math.h"

namespace SpinoCore::Core::Rendering {
    namespace {
        constexpr int CircleSegments = 24;
        constexpr float PlaneHalfSize = 2.0f;
        constexpr Math::Vector4f RestingColor{0.2f, 1.0f, 0.3f, 1.0f};
        constexpr Math::Vector4f CollidingColor{1.0f, 0.25f, 0.2f, 1.0f};
        constexpr Math::Vector4f TriggerColor{1.0f, 0.85f, 0.1f, 1.0f};

        struct Basis final {
            Math::Vector3f u;
            Math::Vector3f v;
        };

        [[nodiscard]] Basis PerpendicularTo(const Math::Vector3f& axis) {
            const Math::Vector3f helper = Math::Operations::Abs(axis.y) < 0.99f ? Math::Vector3f::UnitY() : Math::Vector3f::UnitX();
            const Math::Vector3f u = axis.Cross(helper).Normalized();
            return Basis{.u = u, .v = axis.Cross(u).Normalized()};
        }

        void AddArc(DebugDrawList& list, const Math::Vector3f& center, const Math::Vector3f& first, const Math::Vector3f& second,
                    const float radius, const float from, const float to, const int segments, const Math::Vector4f& color) {
            const float step = (to - from) / static_cast<float>(segments);
            for (int index = 0; index < segments; ++index) {
                const float a = from + step * static_cast<float>(index);
                const float b = a + step;
                AddDebugLine(list,
                    center + (first * Math::Operations::Cos(a) + second * Math::Operations::Sin(a)) * radius,
                    center + (first * Math::Operations::Cos(b) + second * Math::Operations::Sin(b)) * radius, color);
            }
        }

        void AddCircle(DebugDrawList& list, const Math::Vector3f& center, const Basis& basis, const float radius, const Math::Vector4f& color) {
            AddArc(list, center, basis.u, basis.v, radius, 0.0f, Math::Constants::TwoPI<float>, CircleSegments, color);
        }
    }

    void AddDebugLine(DebugDrawList& list, const Math::Vector3f& from, const Math::Vector3f& to, const Math::Vector4f& color) {
        list.lines.push_back(DebugVertex{.position = from, .color = color});
        list.lines.push_back(DebugVertex{.position = to, .color = color});
    }

    void AddDebugBox(DebugDrawList& list, const Physics::OrientedBox& box, const Math::Vector4f& color) {
        std::array<Math::Vector3f, 8> corners{};
        for (std::size_t index = 0; index < corners.size(); ++index) {
            const float x = (index & 1u) ? 1.0f : -1.0f;
            const float y = (index & 2u) ? 1.0f : -1.0f;
            const float z = (index & 4u) ? 1.0f : -1.0f;
            corners[index] = box.center + box.axes[0] * (x * box.halfExtents.x) + box.axes[1] * (y * box.halfExtents.y) + box.axes[2] * (z * box.halfExtents.z);
        }

        for (std::size_t index = 0; index < corners.size(); ++index) {
            for (const std::size_t bit : {1u, 2u, 4u}) {
                if ((index & bit) == 0) AddDebugLine(list, corners[index], corners[index | bit], color);
            }
        }
    }

    void AddDebugSphere(DebugDrawList& list, const Physics::Sphere& sphere, const Math::Vector4f& color) {
        AddCircle(list, sphere.center, Basis{.u = Math::Vector3f::UnitX(), .v = Math::Vector3f::UnitY()}, sphere.radius, color);
        AddCircle(list, sphere.center, Basis{.u = Math::Vector3f::UnitX(), .v = Math::Vector3f::UnitZ()}, sphere.radius, color);
        AddCircle(list, sphere.center, Basis{.u = Math::Vector3f::UnitY(), .v = Math::Vector3f::UnitZ()}, sphere.radius, color);
    }

    void AddDebugCapsule(DebugDrawList& list, const Physics::Capsule& capsule, const Math::Vector4f& color) {
        const Math::Vector3f segment = capsule.end - capsule.start;
        const Math::Vector3f axis = segment.LengthSq() > 0.0f ? segment.Normalized() : Math::Vector3f::UnitY();
        const Basis basis = PerpendicularTo(axis);
        const float radius = capsule.radius;
        constexpr float Pi = Math::Constants::PI<float>;
        constexpr int HalfSegments = CircleSegments / 2;

        AddCircle(list, capsule.start, basis, radius, color);
        AddCircle(list, capsule.end, basis, radius, color);
        for (const Math::Vector3f& side : {basis.u, basis.u * -1.0f, basis.v, basis.v * -1.0f}) {
            AddDebugLine(list, capsule.start + side * radius, capsule.end + side * radius, color);
        }

        AddArc(list, capsule.end, basis.u, axis, radius, 0.0f, Pi, HalfSegments, color);
        AddArc(list, capsule.end, basis.v, axis, radius, 0.0f, Pi, HalfSegments, color);
        AddArc(list, capsule.start, basis.u, axis * -1.0f, radius, 0.0f, Pi, HalfSegments, color);
        AddArc(list, capsule.start, basis.v, axis * -1.0f, radius, 0.0f, Pi, HalfSegments, color);
    }

    void AddDebugPlane(DebugDrawList& list, const Physics::Plane& plane, const Math::Vector3f& around, const Math::Vector4f& color) {
        const Math::Vector3f center = around - plane.normal * (plane.normal.Dot(around) - plane.distance);
        const Basis basis = PerpendicularTo(plane.normal);
        const Math::Vector3f u = basis.u * PlaneHalfSize;
        const Math::Vector3f v = basis.v * PlaneHalfSize;

        AddDebugLine(list, center - u - v, center + u - v, color);
        AddDebugLine(list, center + u - v, center + u + v, color);
        AddDebugLine(list, center + u + v, center - u + v, color);
        AddDebugLine(list, center - u + v, center - u - v, color);
        AddDebugLine(list, center, center + plane.normal, color);
    }

    Math::Vector4f DebugColorOf(const Actors::Components::ColliderComponent& collider) {
        if (collider.IsTrigger()) return TriggerColor;
        return collider.IsColliding() ? CollidingColor : RestingColor;
    }
}
