// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "ColliderComponent.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::Rendering {
    struct DebugDrawList;
}

namespace SpinoCore::Core::Actors::Components {
    /// @brief Settings of a sphere collider.
    struct SphereColliderInput : ColliderInput {
        /// Radius before the actor scale.
        float radius{1.0f};
        /// Center relative to the actor position.
        Math::Vector3f offset{Math::Vector3f::Zero()};
    };

    /// @brief Sphere collider that follows the actor position.
    class SphereColliderComponent : public ColliderComponent {
    public:
        /// @brief Creates a sphere collider.
        /// @param actor Owner of the collider.
        /// @param input Sphere settings.
        explicit SphereColliderComponent(Actor& actor, const SphereColliderInput& input = {});
        ~SphereColliderComponent() override = default;

        /// @brief Adds the sphere outline to the debug shapes.
        /// @param drawList List that receives the shape.
        void GetDebugDrawCommand(Rendering::DebugDrawList& drawList) override;

        /// @brief Changes the radius.
        /// @param radius New radius.
        void SetRadius(const float radius) { mRadius = radius; }
        /// @return The radius.
        [[nodiscard]] float GetRadius() const { return mRadius; }

        /// @brief Changes the center offset.
        /// @param offset New offset from the actor position.
        void SetOffset(const Math::Vector3f& offset) { mOffset = offset; }
        /// @return The center offset.
        [[nodiscard]] const Math::Vector3f& GetOffset() const { return mOffset; }

    private:
        float mRadius{1.0f};
        Math::Vector3f mOffset{Math::Vector3f::Zero()};
    };
}
