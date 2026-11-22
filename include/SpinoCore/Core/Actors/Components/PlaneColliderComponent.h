// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "ColliderComponent.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::Rendering {
    struct DebugDrawList;
}

namespace SpinoCore::Core::Actors::Components {
    struct PlaneColliderInput : ColliderInput {
        Math::Vector3f normal{Math::Vector3f::UnitY()};
        float distanceToOrigin{0.0f};
    };

    class PlaneColliderComponent : public ColliderComponent {
    public:
        explicit PlaneColliderComponent(Actor& actor, const PlaneColliderInput& input = {});

        [[nodiscard]] float GetDistanceToOrigin() const { return mDistanceToOrigin; }
        [[nodiscard]] const Math::Vector3f& GetNormal() const { return mNormal; }

        void GetDebugDrawCommand(Rendering::DebugDrawList& drawList) override;

    private:
        Math::Vector3f mNormal{Math::Vector3f::UnitY()};
        float mDistanceToOrigin{0.0f};
    };
}
