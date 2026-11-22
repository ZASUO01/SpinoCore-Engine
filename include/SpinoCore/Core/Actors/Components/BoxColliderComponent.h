// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "ColliderComponent.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::Rendering {
    struct DebugDrawList;
}

namespace SpinoCore::Core::Actors::Components {
    /// @brief Settings of a box collider.
    struct BoxColliderInput : ColliderInput {
        /// Half size of the box on each axis, before the actor scale.
        Math::Vector3f extent{Math::Vector3f::One()};
    };

    /// @brief Oriented box collider that follows the actor transform.
    class BoxColliderComponent : public ColliderComponent {
    public:
        /// @brief Creates a box collider.
        /// @param actor Owner of the collider.
        /// @param input Box settings.
        BoxColliderComponent(Actor& actor, const BoxColliderInput& input);

        /// @brief Adds the box outline to the debug shapes.
        /// @param drawList List that receives the shape.
        void GetDebugDrawCommand(Rendering::DebugDrawList& drawList) override;

        /// @brief Changes the half size.
        /// @param extent New half size on each axis.
        void SetExtent(const Math::Vector3f extent) { mExtent = extent; }
        /// @return The half size on each axis.
        [[nodiscard]] const Math::Vector3f& GetExtent() const { return mExtent; }

    private:
        Math::Vector3f mExtent{Math::Vector3f::One()};
    };
}
