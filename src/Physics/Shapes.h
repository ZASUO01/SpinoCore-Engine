// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <optional>
#include "OrientedBox.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Physics {
    struct Ray;

    struct Sphere final {
        Math::Vector3f center{Math::Vector3f::Zero()};
        float radius{0.5f};
    };

    struct Capsule final {
        Math::Vector3f start{Math::Vector3f::Zero()};
        Math::Vector3f end{Math::Vector3f::UnitY()};
        float radius{0.5f};
    };

    struct Plane final {
        Math::Vector3f normal{Math::Vector3f::UnitY()};
        float distance{0.0f};
    };

    struct RayIntersection final {
        float distance{0.0f};
        Math::Vector3f normal{Math::Vector3f::UnitY()};
    };

    [[nodiscard]] Math::Vector3f ClosestPointOnSegment(const Math::Vector3f& start, const Math::Vector3f& end, const Math::Vector3f& point);
    [[nodiscard]] Math::Vector3f ClosestPointOnBox(const OrientedBox& box, const Math::Vector3f& point);

    [[nodiscard]] std::optional<Contact> TestSpheres(const Sphere& a, const Sphere& b);
    [[nodiscard]] std::optional<Contact> TestSphereCapsule(const Sphere& a, const Capsule& b);
    [[nodiscard]] std::optional<Contact> TestCapsules(const Capsule& a, const Capsule& b);
    [[nodiscard]] std::optional<Contact> TestBoxSphere(const OrientedBox& a, const Sphere& b);
    [[nodiscard]] std::optional<Contact> TestBoxCapsule(const OrientedBox& a, const Capsule& b);
    [[nodiscard]] std::optional<Contact> TestBoxPlane(const OrientedBox& a, const Plane& b);
    [[nodiscard]] std::optional<Contact> TestSpherePlane(const Sphere& a, const Plane& b);
    [[nodiscard]] std::optional<Contact> TestCapsulePlane(const Capsule& a, const Plane& b);

    [[nodiscard]] std::optional<RayIntersection> IntersectRaySphere(const Ray& ray, const Sphere& sphere);
    [[nodiscard]] std::optional<RayIntersection> IntersectRayBox(const Ray& ray, const OrientedBox& box);
    [[nodiscard]] std::optional<RayIntersection> IntersectRayCapsule(const Ray& ray, const Capsule& capsule);
    [[nodiscard]] std::optional<RayIntersection> IntersectRayPlane(const Ray& ray, const Plane& plane);

    [[nodiscard]] std::optional<Contact> Flipped(const std::optional<Contact>& contact);
}
