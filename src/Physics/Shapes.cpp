// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Shapes.h"
#include <algorithm>
#include <cstddef>
#include <limits>
#include <utility>
#include "SpinoCore/Math/Math.h"
#include "SpinoCore/Physics/Ray.h"

namespace SpinoCore::Physics {
    namespace {
        constexpr float Epsilon = 1e-6f;
        constexpr int BoxCapsuleIterations = 4;

        [[nodiscard]] float Component(const Math::Vector3f& vector, const std::size_t index) {
            if (index == 0) return vector.x;
            if (index == 1) return vector.y;
            return vector.z;
        }

        [[nodiscard]] std::optional<Contact> TestPoints(const Math::Vector3f& a, const float radiusA, const Math::Vector3f& b, const float radiusB) {
            const Math::Vector3f offset = b - a;
            const float distanceSq = offset.LengthSq();
            const float reach = radiusA + radiusB;
            if (distanceSq >= reach * reach) return std::nullopt;

            const float distance = Math::Operations::Sqrt(distanceSq);
            const Math::Vector3f normal = distance > Epsilon ? offset * (1.0f / distance) : Math::Vector3f::UnitY();
            return Contact{.normal = normal, .depth = reach - distance};
        }

        [[nodiscard]] float SignedDistance(const Plane& plane, const Math::Vector3f& point) {
            return plane.normal.Dot(point) - plane.distance;
        }

        [[nodiscard]] std::optional<Contact> AgainstPlane(const Plane& plane, const float closestDistance, const float radius) {
            const float depth = radius - closestDistance;
            if (depth <= 0.0f) return std::nullopt;
            return Contact{.normal = plane.normal * -1.0f, .depth = depth};
        }

        void ClosestPointsBetweenSegments(const Capsule& a, const Capsule& b, Math::Vector3f& onA, Math::Vector3f& onB) {
            const Math::Vector3f directionA = a.end - a.start;
            const Math::Vector3f directionB = b.end - b.start;
            const Math::Vector3f offset = a.start - b.start;
            const float lengthA = directionA.LengthSq();
            const float lengthB = directionB.LengthSq();
            const float f = directionB.Dot(offset);

            float s = 0.0f;
            float t = 0.0f;
            if (lengthA <= Epsilon && lengthB <= Epsilon) {
                onA = a.start;
                onB = b.start;
                return;
            }
            if (lengthA <= Epsilon) {
                t = std::clamp(f / lengthB, 0.0f, 1.0f);
            } else {
                const float c = directionA.Dot(offset);
                if (lengthB <= Epsilon) {
                    s = std::clamp(-c / lengthA, 0.0f, 1.0f);
                } else {
                    const float b2 = directionA.Dot(directionB);
                    const float denominator = lengthA * lengthB - b2 * b2;
                    s = denominator > Epsilon ? std::clamp((b2 * f - c * lengthB) / denominator, 0.0f, 1.0f) : 0.0f;
                    t = (b2 * s + f) / lengthB;
                    if (t < 0.0f) {
                        t = 0.0f;
                        s = std::clamp(-c / lengthA, 0.0f, 1.0f);
                    } else if (t > 1.0f) {
                        t = 1.0f;
                        s = std::clamp((b2 - c) / lengthA, 0.0f, 1.0f);
                    }
                }
            }
            onA = a.start + directionA * s;
            onB = b.start + directionB * t;
        }
    }

    Math::Vector3f ClosestPointOnSegment(const Math::Vector3f& start, const Math::Vector3f& end, const Math::Vector3f& point) {
        const Math::Vector3f direction = end - start;
        const float lengthSq = direction.LengthSq();
        if (lengthSq <= Epsilon) return start;
        return start + direction * std::clamp((point - start).Dot(direction) / lengthSq, 0.0f, 1.0f);
    }

    Math::Vector3f ClosestPointOnBox(const OrientedBox& box, const Math::Vector3f& point) {
        const Math::Vector3f offset = point - box.center;
        Math::Vector3f closest = box.center;
        for (std::size_t index = 0; index < 3; ++index) {
            const float extent = Component(box.halfExtents, index);
            closest += box.axes[index] * std::clamp(offset.Dot(box.axes[index]), -extent, extent);
        }
        return closest;
    }

    std::optional<Contact> TestSpheres(const Sphere& a, const Sphere& b) {
        return TestPoints(a.center, a.radius, b.center, b.radius);
    }

    std::optional<Contact> TestSphereCapsule(const Sphere& a, const Capsule& b) {
        return TestPoints(a.center, a.radius, ClosestPointOnSegment(b.start, b.end, a.center), b.radius);
    }

    std::optional<Contact> TestCapsules(const Capsule& a, const Capsule& b) {
        Math::Vector3f onA{};
        Math::Vector3f onB{};
        ClosestPointsBetweenSegments(a, b, onA, onB);
        return TestPoints(onA, a.radius, onB, b.radius);
    }

    std::optional<Contact> TestBoxSphere(const OrientedBox& a, const Sphere& b) {
        const Math::Vector3f closest = ClosestPointOnBox(a, b.center);
        const Math::Vector3f outward = b.center - closest;
        const float distanceSq = outward.LengthSq();

        if (distanceSq > Epsilon) {
            if (distanceSq >= b.radius * b.radius) return std::nullopt;
            const float distance = Math::Operations::Sqrt(distanceSq);
            return Contact{.normal = outward * (1.0f / distance), .depth = b.radius - distance};
        }

        const Math::Vector3f offset = b.center - a.center;
        Contact contact{.normal = Math::Vector3f::UnitY(), .depth = std::numeric_limits<float>::max()};
        for (std::size_t index = 0; index < 3; ++index) {
            const float along = offset.Dot(a.axes[index]);
            const float toFace = Component(a.halfExtents, index) - Math::Operations::Abs(along);
            if (toFace < contact.depth) {
                contact.depth = toFace;
                contact.normal = along < 0.0f ? a.axes[index] * -1.0f : a.axes[index];
            }
        }
        contact.depth += b.radius;
        return contact;
    }

    std::optional<Contact> TestBoxCapsule(const OrientedBox& a, const Capsule& b) {
        Math::Vector3f onSegment = ClosestPointOnSegment(b.start, b.end, a.center);
        for (int iteration = 0; iteration < BoxCapsuleIterations; ++iteration) {
            onSegment = ClosestPointOnSegment(b.start, b.end, ClosestPointOnBox(a, onSegment));
        }
        return TestBoxSphere(a, Sphere{.center = onSegment, .radius = b.radius});
    }

    std::optional<Contact> TestBoxPlane(const OrientedBox& a, const Plane& b) {
        float radius = 0.0f;
        for (std::size_t index = 0; index < 3; ++index) {
            radius += Component(a.halfExtents, index) * Math::Operations::Abs(a.axes[index].Dot(b.normal));
        }
        return AgainstPlane(b, SignedDistance(b, a.center), radius);
    }

    std::optional<Contact> TestSpherePlane(const Sphere& a, const Plane& b) {
        return AgainstPlane(b, SignedDistance(b, a.center), a.radius);
    }

    std::optional<Contact> TestCapsulePlane(const Capsule& a, const Plane& b) {
        return AgainstPlane(b, std::min(SignedDistance(b, a.start), SignedDistance(b, a.end)), a.radius);
    }

    std::optional<Contact> Flipped(const std::optional<Contact>& contact) {
        if (!contact) return std::nullopt;
        return Contact{.normal = contact->normal * -1.0f, .depth = contact->depth};
    }
    std::optional<RayIntersection> IntersectRaySphere(const Ray& ray, const Sphere& sphere) {
        const Math::Vector3f offset = ray.origin - sphere.center;
        const float along = offset.Dot(ray.direction);
        const float outside = offset.LengthSq() - sphere.radius * sphere.radius;
        if (outside <= 0.0f) return RayIntersection{.distance = 0.0f, .normal = ray.direction * -1.0f};
        if (along > 0.0f) return std::nullopt;

        const float discriminant = along * along - outside;
        if (discriminant < 0.0f) return std::nullopt;

        const float distance = -along - Math::Operations::Sqrt(discriminant);
        const Math::Vector3f normal = (ray.PointAt(distance) - sphere.center).Normalized();
        return RayIntersection{.distance = distance, .normal = normal};
    }

    std::optional<RayIntersection> IntersectRayBox(const Ray& ray, const OrientedBox& box) {
        const Math::Vector3f offset = ray.origin - box.center;
        float entry = 0.0f;
        float exit = std::numeric_limits<float>::max();
        Math::Vector3f normal = ray.direction * -1.0f;

        for (std::size_t index = 0; index < 3; ++index) {
            const Math::Vector3f& axis = box.axes[index];
            const float halfExtent = Component(box.halfExtents, index);
            const float start = offset.Dot(axis);
            const float speed = ray.direction.Dot(axis);

            if (Math::Operations::Abs(speed) <= Epsilon) {
                if (Math::Operations::Abs(start) > halfExtent) return std::nullopt;
                continue;
            }

            float nearSide = (-halfExtent - start) / speed;
            float farSide = (halfExtent - start) / speed;
            if (nearSide > farSide) std::swap(nearSide, farSide);

            if (nearSide > entry) {
                entry = nearSide;
                normal = speed > 0.0f ? axis * -1.0f : axis;
            }
            exit = std::min(exit, farSide);
            if (entry > exit) return std::nullopt;
        }

        return RayIntersection{.distance = entry, .normal = normal};
    }

    std::optional<RayIntersection> IntersectRayCapsule(const Ray& ray, const Capsule& capsule) {
        std::optional<RayIntersection> nearest{};
        const auto keepNearest = [&nearest](const std::optional<RayIntersection>& candidate) {
            if (candidate && (!nearest || candidate->distance < nearest->distance)) nearest = candidate;
        };

        keepNearest(IntersectRaySphere(ray, Sphere{.center = capsule.start, .radius = capsule.radius}));
        keepNearest(IntersectRaySphere(ray, Sphere{.center = capsule.end, .radius = capsule.radius}));

        const Math::Vector3f segment = capsule.end - capsule.start;
        const float length = segment.Length();
        if (length <= Epsilon) return nearest;

        const Math::Vector3f axis = segment * (1.0f / length);
        const Math::Vector3f offset = ray.origin - capsule.start;
        const Math::Vector3f offsetAcross = offset - axis * offset.Dot(axis);
        const Math::Vector3f directionAcross = ray.direction - axis * ray.direction.Dot(axis);
        const float outside = offsetAcross.LengthSq() - capsule.radius * capsule.radius;
        const float height = offset.Dot(axis);

        if (outside <= 0.0f && height >= 0.0f && height <= length) {
            return RayIntersection{.distance = 0.0f, .normal = ray.direction * -1.0f};
        }

        const float a = directionAcross.LengthSq();
        if (a <= Epsilon) return nearest;

        const float b = offsetAcross.Dot(directionAcross);
        const float discriminant = b * b - a * outside;
        if (discriminant < 0.0f) return nearest;

        const float distance = (-b - Math::Operations::Sqrt(discriminant)) / a;
        if (distance < 0.0f) return nearest;

        const Math::Vector3f point = ray.PointAt(distance);
        const float pointHeight = (point - capsule.start).Dot(axis);
        if (pointHeight < 0.0f || pointHeight > length) return nearest;

        const Math::Vector3f normal = (point - (capsule.start + axis * pointHeight)).Normalized();
        keepNearest(RayIntersection{.distance = distance, .normal = normal});
        return nearest;
    }

    std::optional<RayIntersection> IntersectRayPlane(const Ray& ray, const Plane& plane) {
        const float speed = plane.normal.Dot(ray.direction);
        if (Math::Operations::Abs(speed) <= Epsilon) return std::nullopt;

        const float distance = -SignedDistance(plane, ray.origin) / speed;
        if (distance < 0.0f) return std::nullopt;

        return RayIntersection{.distance = distance, .normal = speed < 0.0f ? plane.normal : plane.normal * -1.0f};
    }
}
