// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "OrientedBox.h"
#include <cstddef>
#include <limits>
#include <vector>
#include "SpinoCore/Math/Math.h"

namespace SpinoCore::Physics {
    namespace {
        constexpr float DegenerateAxis = 1e-6f;

        [[nodiscard]] float Component(const Math::Vector3f& vector, const std::size_t index) {
            return index == 0 ? vector.x : index == 1 ? vector.y : vector.z;
        }

        [[nodiscard]] float ProjectedRadius(const OrientedBox& box, const Math::Vector3f& axis) {
            float radius = 0.0f;
            for (std::size_t index = 0; index < 3; ++index) {
                radius += Component(box.halfExtents, index) * Math::Operations::Abs(box.axes[index].Dot(axis));
            }
            return radius;
        }

        [[nodiscard]] std::vector<Math::Vector3f> SeparatingAxes(const OrientedBox& a, const OrientedBox& b) {
            std::vector<Math::Vector3f> axes(a.axes.begin(), a.axes.end());
            axes.insert(axes.end(), b.axes.begin(), b.axes.end());
            for (const auto& axisA : a.axes) {
                for (const auto& axisB : b.axes) {
                    if (const Math::Vector3f cross = axisA.Cross(axisB); cross.LengthSq() > DegenerateAxis) {
                        axes.push_back(cross.Normalized());
                    }
                }
            }
            return axes;
        }
    }

    OrientedBox MakeOrientedBox(const Math::Matrix4f& world, const Math::Vector3f& halfExtents) {
        OrientedBox box{};
        box.center = world.TransformPoint(Math::Vector3f::Zero());

        float scales[3]{};
        for (std::size_t column = 0; column < 3; ++column) {
            const Math::Vector3f axis{world(0, column), world(1, column), world(2, column)};
            scales[column] = axis.Length();
            if (scales[column] > DegenerateAxis) box.axes[column] = axis * (1.0f / scales[column]);
        }

        box.halfExtents = {halfExtents.x * scales[0], halfExtents.y * scales[1], halfExtents.z * scales[2]};
        return box;
    }

    std::optional<Contact> TestOrientedBoxes(const OrientedBox& a, const OrientedBox& b) {
        const Math::Vector3f offset = b.center - a.center;
        Contact best{.normal = Math::Vector3f::UnitX(), .depth = std::numeric_limits<float>::max()};

        for (const Math::Vector3f& axis : SeparatingAxes(a, b)) {
            const float distance = offset.Dot(axis);
            const float overlap = ProjectedRadius(a, axis) + ProjectedRadius(b, axis) - Math::Operations::Abs(distance);
            if (overlap <= 0.0f) return std::nullopt;

            if (overlap < best.depth) {
                best.depth = overlap;
                best.normal = distance < 0.0f ? axis * -1.0f : axis;
            }
        }
        return best;
    }
}
