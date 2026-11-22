// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "ColliderComponent.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::Rendering {
    struct DebugDrawList;
}

namespace SpinoCore::Core::Actors::Components {
    struct SphereColliderInput : ColliderInput {
        float radius{1.0f};
        Math::Vector3f offset{Math::Vector3f::Zero()};
    };

    class SphereColliderComponent : public ColliderComponent {
    public:
        explicit SphereColliderComponent(Actor& actor, const SphereColliderInput& input = {});
        ~SphereColliderComponent() override = default;

        void GetDebugDrawCommand(Rendering::DebugDrawList& drawList) override;

        void SetRadius(const float radius) { mRadius = radius; }
        [[nodiscard]] float GetRadius() const { return mRadius; }

        void SetOffset(const Math::Vector3f& offset) { mOffset = offset; }
        [[nodiscard]] const Math::Vector3f& GetOffset() const { return mOffset; }

    private:
        float mRadius{1.0f};
        Math::Vector3f mOffset{Math::Vector3f::Zero()};
    };
}
