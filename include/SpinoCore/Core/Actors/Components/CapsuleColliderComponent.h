// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "ColliderComponent.h"

namespace SpinoCore::Core::Rendering {
    struct DebugDrawList;
}

namespace SpinoCore::Core::Actors::Components {
    /// @brief Settings of a capsule collider.
    struct CapsuleColliderInput : ColliderInput {
        /// Radius of the capsule.
        float radius{0.5f};
        /// Half the length of the segment between the two sphere centers, along the local Y axis.
        float halfHeight{0.5f};
    };

    /// @brief Vertical capsule collider that follows the actor transform.
    class CapsuleColliderComponent : public ColliderComponent {
    public:
        /// @brief Creates a capsule collider.
        /// @param actor Owner of the collider.
        /// @param input Capsule settings.
        explicit CapsuleColliderComponent(Actor& actor, const CapsuleColliderInput& input = {});

        /// @brief Adds the capsule outline to the debug shapes.
        /// @param drawList List that receives the shape.
        void GetDebugDrawCommand(Rendering::DebugDrawList& drawList) override;

        /// @brief Changes the radius.
        /// @param radius New radius.
        void SetRadius(const float radius) { mRadius = radius; }
        /// @return The radius.
        [[nodiscard]] float GetRadius() const { return mRadius; }

        /// @brief Changes the half length of the inner segment.
        /// @param halfHeight New half length.
        void SetHalfHeight(const float halfHeight) { mHalfHeight = halfHeight; }
        /// @return The half length of the inner segment.
        [[nodiscard]] float GetHalfHeight() const { return mHalfHeight; }

    private:
        float mRadius{0.5f};
        float mHalfHeight{0.5f};
    };
}
