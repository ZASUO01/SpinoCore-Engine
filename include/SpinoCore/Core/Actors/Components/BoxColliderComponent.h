// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "ColliderComponent.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::Rendering {
    struct DebugDrawList;
}

namespace SpinoCore::Core::Actors::Components {
    struct BoxColliderInput : ColliderInput {
        Math::Vector3f extent{Math::Vector3f::One()};
    };

    class BoxColliderComponent : public ColliderComponent {
    public:
        BoxColliderComponent(Actor& actor, const BoxColliderInput& input);

        void GetDebugDrawCommand(Rendering::DebugDrawList& drawList) override;

        void SetExtent(const Math::Vector3f extent) { mExtent = extent; }
        [[nodiscard]] const Math::Vector3f& GetExtent() const { return mExtent; }

    private:
        Math::Vector3f mExtent{Math::Vector3f::One()};
    };
}
