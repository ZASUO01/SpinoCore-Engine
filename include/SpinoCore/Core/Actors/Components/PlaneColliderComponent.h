// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "ColliderComponent.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::Rendering {
    struct DebugDrawList;
}

namespace SpinoCore::Core::Actors::Components {
    /// @brief Settings of an infinite plane collider.
    struct PlaneColliderInput : ColliderInput {
        /// Direction the plane faces.
        Math::Vector3f normal{Math::Vector3f::UnitY()};
        /// Signed distance from the world origin along the normal.
        float distanceToOrigin{0.0f};
    };

    /// @brief Infinite static plane, usually the ground.
    class PlaneColliderComponent : public ColliderComponent {
    public:
        /// @brief Creates a static plane collider.
        /// @param actor Owner of the collider.
        /// @param input Plane settings.
        explicit PlaneColliderComponent(Actor& actor, const PlaneColliderInput& input = {});

        /// @return The signed distance from the world origin.
        [[nodiscard]] float GetDistanceToOrigin() const { return mDistanceToOrigin; }
        /// @return The direction the plane faces.
        [[nodiscard]] const Math::Vector3f& GetNormal() const { return mNormal; }

        /// @brief Adds a patch of the plane to the debug shapes.
        /// @param drawList List that receives the shape.
        void GetDebugDrawCommand(Rendering::DebugDrawList& drawList) override;

    private:
        Math::Vector3f mNormal{Math::Vector3f::UnitY()};
        float mDistanceToOrigin{0.0f};
    };
}
