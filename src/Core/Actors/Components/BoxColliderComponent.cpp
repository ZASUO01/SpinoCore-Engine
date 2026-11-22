// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/Actors/Components/BoxColliderComponent.h"
#include "Core/Rendering/Drawer/DebugShapes.h"
#include "Physics/ColliderShapes.h"

namespace SpinoCore::Core::Actors::Components {
    namespace {
        [[nodiscard]] ColliderInput AsBox(const ColliderInput& base) {
            ColliderInput input = base;
            input.type = ColliderType::Box;
            return input;
        }
    }

    BoxColliderComponent::BoxColliderComponent(Actor& actor, const BoxColliderInput& input)
    :ColliderComponent(actor, AsBox(input))
    ,mExtent(input.extent)
    {}

    void BoxColliderComponent::GetDebugDrawCommand(Rendering::DebugDrawList& drawList) {
        Rendering::AddDebugBox(drawList, Physics::BoxOf(*this), Rendering::DebugColorOf(*this));
    }
}
