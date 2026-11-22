// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/Actors/Components/SphereColliderComponent.h"
#include "Core/Rendering/Drawer/DebugShapes.h"
#include "Physics/ColliderShapes.h"

namespace SpinoCore::Core::Actors::Components {
    namespace {
        [[nodiscard]] ColliderInput AsSphere(ColliderInput input) {
            input.type = ColliderType::Sphere;
            return input;
        }
    }

    SphereColliderComponent::SphereColliderComponent(Actor& actor, const SphereColliderInput& input)
    :ColliderComponent(actor, AsSphere(input))
    ,mRadius(input.radius)
    ,mOffset(input.offset)
    {}

    void SphereColliderComponent::GetDebugDrawCommand(Rendering::DebugDrawList& drawList) {
        Rendering::AddDebugSphere(drawList, Physics::SphereOf(*this), Rendering::DebugColorOf(*this));
    }
}
