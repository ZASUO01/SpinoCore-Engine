// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "CollisionProcessor.h"
#include <algorithm>
#include <optional>
#include "ColliderShapes.h"
#include "OrientedBox.h"
#include "Shapes.h"
#include "SpinoCore/Core/Actors/Actor.h"
#include "SpinoCore/Core/Actors/Components/BoxColliderComponent.h"
#include "SpinoCore/Core/Actors/Components/CapsuleColliderComponent.h"
#include "SpinoCore/Core/Actors/Components/ColliderComponent.h"
#include "SpinoCore/Core/Actors/Components/PlaneColliderComponent.h"
#include "SpinoCore/Core/Actors/Components/RigidBodyComponent.h"
#include "SpinoCore/Core/Actors/Components/SphereColliderComponent.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Physics {
    namespace {
        using Core::Actors::Components::RigidBodyComponent;

        [[nodiscard]] RigidBodyComponent* MovingBodyOf(const ColliderComponent& collider) {
            return collider.IsStatic() ? nullptr : collider.GetActor()->GetComponent<RigidBodyComponent>();
        }

        [[nodiscard]] float PositionWeightOf(const ColliderComponent& collider) {
            if (collider.IsStatic()) return 0.0f;
            const RigidBodyComponent* body = MovingBodyOf(collider);
            return body ? body->GetInverseMass() : 1.0f;
        }

        [[nodiscard]] const BoxColliderComponent& AsBox(const ColliderComponent* collider) {
            return *static_cast<const BoxColliderComponent*>(collider);
        }

        [[nodiscard]] const SphereColliderComponent& AsSphere(const ColliderComponent* collider) {
            return *static_cast<const SphereColliderComponent*>(collider);
        }

        [[nodiscard]] const CapsuleColliderComponent& AsCapsule(const ColliderComponent* collider) {
            return *static_cast<const CapsuleColliderComponent*>(collider);
        }

        [[nodiscard]] const PlaneColliderComponent& AsPlane(const ColliderComponent* collider) {
            return *static_cast<const PlaneColliderComponent*>(collider);
        }

        void Move(const ColliderComponent& collider, const Math::Vector3f& offset) {
            Core::Actors::Actor* actor = collider.GetActor();
            actor->SetPosition(actor->GetPosition() + offset);
        }
    }

    void CollisionProcessor::RegisterCollider(ColliderComponent *collider) {
        if (!collider) return;

        auto& colliders = collider->IsStatic() ? mStaticColliders : mDynamicColliders;
        if (std::ranges::find(colliders, collider) == colliders.end()) colliders.push_back(collider);
    }

    void CollisionProcessor::UnregisterCollider(ColliderComponent *collider) {
        if (!collider) return;

        std::erase(collider->IsStatic() ? mStaticColliders : mDynamicColliders, collider);
    }

    void CollisionProcessor::Update() {
        for (ColliderComponent* collider : mDynamicColliders) collider->mIsColliding = false;
        for (ColliderComponent* collider : mStaticColliders) collider->mIsColliding = false;

        for (std::size_t i = 0; i < mDynamicColliders.size(); ++i) {
            for (std::size_t j = i + 1; j < mDynamicColliders.size(); ++j) {
                TestCollision(mDynamicColliders[i], mDynamicColliders[j]);
            }
        }

        for (ColliderComponent* dynamicCollider : mDynamicColliders) {
            for (ColliderComponent* staticCollider : mStaticColliders) {
                TestCollision(dynamicCollider, staticCollider);
            }
        }
    }

    void CollisionProcessor::TestCollision(ColliderComponent *a, ColliderComponent *b) {
        using Core::Actors::Components::ColliderType;

        if (!a->IsEnabled() || !b->IsEnabled()) return;

        std::optional<Contact> contact{};
        switch (a->GetType()) {
            case ColliderType::Box:
                switch (b->GetType()) {
                    case ColliderType::Box: contact = CheckBoxBoxCollision(AsBox(a), AsBox(b)); break;
                    case ColliderType::Sphere: contact = CheckBoxSphereCollision(AsBox(a), AsSphere(b)); break;
                    case ColliderType::Capsule: contact = CheckBoxCapsuleCollision(AsBox(a), AsCapsule(b)); break;
                    case ColliderType::Plane: contact = CheckBoxPlaneCollision(AsBox(a), AsPlane(b)); break;
                }
                break;
            case ColliderType::Sphere:
                switch (b->GetType()) {
                    case ColliderType::Box: contact = Flipped(CheckBoxSphereCollision(AsBox(b), AsSphere(a))); break;
                    case ColliderType::Sphere: contact = CheckSphereSphereCollision(AsSphere(a), AsSphere(b)); break;
                    case ColliderType::Capsule: contact = CheckSphereCapsuleCollision(AsSphere(a), AsCapsule(b)); break;
                    case ColliderType::Plane: contact = CheckSpherePlaneCollision(AsSphere(a), AsPlane(b)); break;
                }
                break;
            case ColliderType::Capsule:
                switch (b->GetType()) {
                    case ColliderType::Box: contact = Flipped(CheckBoxCapsuleCollision(AsBox(b), AsCapsule(a))); break;
                    case ColliderType::Sphere: contact = Flipped(CheckSphereCapsuleCollision(AsSphere(b), AsCapsule(a))); break;
                    case ColliderType::Capsule: contact = CheckCapsuleCapsuleCollision(AsCapsule(a), AsCapsule(b)); break;
                    case ColliderType::Plane: contact = CheckCapsulePlaneCollision(AsCapsule(a), AsPlane(b)); break;
                }
                break;
            case ColliderType::Plane:
                switch (b->GetType()) {
                    case ColliderType::Box: contact = Flipped(CheckBoxPlaneCollision(AsBox(b), AsPlane(a))); break;
                    case ColliderType::Sphere: contact = Flipped(CheckSpherePlaneCollision(AsSphere(b), AsPlane(a))); break;
                    case ColliderType::Capsule: contact = Flipped(CheckCapsulePlaneCollision(AsCapsule(b), AsPlane(a))); break;
                    case ColliderType::Plane: break;
                }
                break;
        }

        if (contact) ResolveCollision(*a, *b, *contact);
    }

    std::optional<Contact> CollisionProcessor::CheckBoxBoxCollision(const BoxColliderComponent& a, const BoxColliderComponent& b) {
        return TestOrientedBoxes(BoxOf(a), BoxOf(b));
    }

    std::optional<Contact> CollisionProcessor::CheckBoxSphereCollision(const BoxColliderComponent& a, const SphereColliderComponent& b) {
        return TestBoxSphere(BoxOf(a), SphereOf(b));
    }

    std::optional<Contact> CollisionProcessor::CheckBoxCapsuleCollision(const BoxColliderComponent& a, const CapsuleColliderComponent& b) {
        return TestBoxCapsule(BoxOf(a), CapsuleOf(b));
    }

    std::optional<Contact> CollisionProcessor::CheckBoxPlaneCollision(const BoxColliderComponent& a, const PlaneColliderComponent& b) {
        return TestBoxPlane(BoxOf(a), PlaneOf(b));
    }

    std::optional<Contact> CollisionProcessor::CheckSphereSphereCollision(const SphereColliderComponent& a, const SphereColliderComponent& b) {
        return TestSpheres(SphereOf(a), SphereOf(b));
    }

    std::optional<Contact> CollisionProcessor::CheckSphereCapsuleCollision(const SphereColliderComponent& a, const CapsuleColliderComponent& b) {
        return TestSphereCapsule(SphereOf(a), CapsuleOf(b));
    }

    std::optional<Contact> CollisionProcessor::CheckSpherePlaneCollision(const SphereColliderComponent& a, const PlaneColliderComponent& b) {
        return TestSpherePlane(SphereOf(a), PlaneOf(b));
    }

    std::optional<Contact> CollisionProcessor::CheckCapsuleCapsuleCollision(const CapsuleColliderComponent& a, const CapsuleColliderComponent& b) {
        return TestCapsules(CapsuleOf(a), CapsuleOf(b));
    }

    std::optional<Contact> CollisionProcessor::CheckCapsulePlaneCollision(const CapsuleColliderComponent& a, const PlaneColliderComponent& b) {
        return TestCapsulePlane(CapsuleOf(a), PlaneOf(b));
    }

    void CollisionProcessor::ResolveCollision(ColliderComponent& a, ColliderComponent& b, const Contact& contact) {
        a.mIsColliding = true;
        b.mIsColliding = true;
        a.OnCollisionCallBack(b);
        b.OnCollisionCallBack(a);

        if (a.IsTrigger() || b.IsTrigger()) return;
        Separate(a, b, contact);
    }

    void CollisionProcessor::Separate(ColliderComponent& a, ColliderComponent& b, const Contact& contact) {
        const float weightA = PositionWeightOf(a);
        const float weightB = PositionWeightOf(b);
        const float totalWeight = weightA + weightB;
        if (totalWeight <= 0.0f) return;

        Move(a, contact.normal * (-contact.depth * weightA / totalWeight));
        Move(b, contact.normal * (contact.depth * weightB / totalWeight));

        RigidBodyComponent* bodyA = MovingBodyOf(a);
        RigidBodyComponent* bodyB = MovingBodyOf(b);
        const float inverseMassA = bodyA ? bodyA->GetInverseMass() : 0.0f;
        const float inverseMassB = bodyB ? bodyB->GetInverseMass() : 0.0f;
        if (inverseMassA + inverseMassB <= 0.0f) return;

        const Math::Vector3f velocityA = bodyA ? bodyA->GetVelocity() : Math::Vector3f::Zero();
        const Math::Vector3f velocityB = bodyB ? bodyB->GetVelocity() : Math::Vector3f::Zero();
        const float approachSpeed = (velocityB - velocityA).Dot(contact.normal);
        if (approachSpeed >= 0.0f) return;

        const float minimumBounceSpeed = (a.GetMinRestitutionVelocity() + b.GetMinRestitutionVelocity()) * 0.5f;
        const float restitution = -approachSpeed < minimumBounceSpeed ? 0.0f : (a.GetRestitution() + b.GetRestitution()) * 0.5f;
        const float impulse = -(1.0f + restitution) * approachSpeed / (inverseMassA + inverseMassB);

        if (bodyA) bodyA->ApplyImpulse(contact.normal * -impulse);
        if (bodyB) bodyB->ApplyImpulse(contact.normal * impulse);
    }
}
