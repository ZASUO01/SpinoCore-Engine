// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/Actors/Components/CapsuleColliderComponent.h"
#include "Core/Rendering/Drawer/DebugShapes.h"
#include "Physics/ColliderShapes.h"

namespace SpinoCore::Core::Actors::Components {
    namespace {
        [[nodiscard]] ColliderInput AsCapsule(ColliderInput input) {
            input.type = ColliderType::Capsule;
            return input;
        }
    }

    CapsuleColliderComponent::CapsuleColliderComponent(Actor& actor, const CapsuleColliderInput& input)
    :ColliderComponent(actor, AsCapsule(input))
    ,mRadius(input.radius)
    ,mHalfHeight(input.halfHeight)
    {}

    void CapsuleColliderComponent::GetDebugDrawCommand(Rendering::DebugDrawList& drawList) {
        Rendering::AddDebugCapsule(drawList, Physics::CapsuleOf(*this), Rendering::DebugColorOf(*this));
    }
}
