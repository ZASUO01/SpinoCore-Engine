// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/Actors/Components/PlaneColliderComponent.h"
#include "Core/Rendering/Drawer/DebugShapes.h"
#include "Physics/ColliderShapes.h"
#include "SpinoCore/Core/Actors/Actor.h"

namespace SpinoCore::Core::Actors::Components {
    namespace {
        [[nodiscard]] ColliderInput AsStaticPlane(const ColliderInput& base) {
            ColliderInput input = base;
            input.type = ColliderType::Plane;
            input.isStatic = true;
            return input;
        }
    }

    PlaneColliderComponent::PlaneColliderComponent(Actor& actor, const PlaneColliderInput& input)
    :ColliderComponent(actor, AsStaticPlane(input))
    ,mNormal(input.normal)
    ,mDistanceToOrigin(input.distanceToOrigin)
    {}

    void PlaneColliderComponent::GetDebugDrawCommand(Rendering::DebugDrawList& drawList) {
        const Math::Vector3f around = GetActor()->GetWorldTransform().TransformPoint(Math::Vector3f::Zero());
        Rendering::AddDebugPlane(drawList, Physics::PlaneOf(*this), around, Rendering::DebugColorOf(*this));
    }
}
