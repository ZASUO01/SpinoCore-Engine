// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "ColliderComponent.h"

namespace SpinoCore::Core::Rendering {
    struct DebugDrawList;
}

namespace SpinoCore::Core::Actors::Components {
    struct CapsuleColliderInput : ColliderInput {
        float radius{0.5f};
        float halfHeight{0.5f};
    };

    class CapsuleColliderComponent : public ColliderComponent {
    public:
        explicit CapsuleColliderComponent(Actor& actor, const CapsuleColliderInput& input = {});

        void GetDebugDrawCommand(Rendering::DebugDrawList& drawList) override;

        void SetRadius(const float radius) { mRadius = radius; }
        [[nodiscard]] float GetRadius() const { return mRadius; }

        void SetHalfHeight(const float halfHeight) { mHalfHeight = halfHeight; }
        [[nodiscard]] float GetHalfHeight() const { return mHalfHeight; }

    private:
        float mRadius{0.5f};
        float mHalfHeight{0.5f};
    };
}
