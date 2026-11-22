// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstddef>
#include <optional>
#include <vector>
#include "OrientedBox.h"
#include "SpinoCore/Physics/Ray.h"

namespace SpinoCore::Core::Actors::Components {
    class BoxColliderComponent;
    class CapsuleColliderComponent;
    class ColliderComponent;
    class PlaneColliderComponent;
    class SphereColliderComponent;
}

namespace SpinoCore::Physics {
    using Core::Actors::Components::BoxColliderComponent;
    using Core::Actors::Components::CapsuleColliderComponent;
    using Core::Actors::Components::ColliderComponent;
    using Core::Actors::Components::PlaneColliderComponent;
    using Core::Actors::Components::SphereColliderComponent;

    class CollisionProcessor final {
    public:
        CollisionProcessor() = default;
        ~CollisionProcessor() = default;

        CollisionProcessor(const CollisionProcessor&) = delete;
        CollisionProcessor& operator=(const CollisionProcessor&) = delete;
        CollisionProcessor(CollisionProcessor&&) = delete;
        CollisionProcessor& operator=(CollisionProcessor&&) = delete;

        void RegisterCollider(ColliderComponent* collider);
        void UnregisterCollider(ColliderComponent* collider);
        void Update();
        [[nodiscard]] std::optional<RaycastHit> Raycast(const Ray& ray, float maxDistance, const RaycastFilter& filter = {}) const;

        [[nodiscard]] std::size_t GetColliderCount() const noexcept { return mDynamicColliders.size() + mStaticColliders.size(); }

    private:
        void TestCollision(ColliderComponent* a, ColliderComponent* b);

        [[nodiscard]] static std::optional<Contact> CheckBoxBoxCollision(const BoxColliderComponent& a, const BoxColliderComponent& b);
        [[nodiscard]] static std::optional<Contact> CheckBoxSphereCollision(const BoxColliderComponent& a, const SphereColliderComponent& b);
        [[nodiscard]] static std::optional<Contact> CheckBoxCapsuleCollision(const BoxColliderComponent& a, const CapsuleColliderComponent& b);
        [[nodiscard]] static std::optional<Contact> CheckBoxPlaneCollision(const BoxColliderComponent& a, const PlaneColliderComponent& b);
        [[nodiscard]] static std::optional<Contact> CheckSphereSphereCollision(const SphereColliderComponent& a, const SphereColliderComponent& b);
        [[nodiscard]] static std::optional<Contact> CheckSphereCapsuleCollision(const SphereColliderComponent& a, const CapsuleColliderComponent& b);
        [[nodiscard]] static std::optional<Contact> CheckSpherePlaneCollision(const SphereColliderComponent& a, const PlaneColliderComponent& b);
        [[nodiscard]] static std::optional<Contact> CheckCapsuleCapsuleCollision(const CapsuleColliderComponent& a, const CapsuleColliderComponent& b);
        [[nodiscard]] static std::optional<Contact> CheckCapsulePlaneCollision(const CapsuleColliderComponent& a, const PlaneColliderComponent& b);

        static void ResolveCollision(ColliderComponent& a, ColliderComponent& b, const Contact& contact);
        static void Separate(ColliderComponent& a, ColliderComponent& b, const Contact& contact);

        std::vector<ColliderComponent*> mDynamicColliders;
        std::vector<ColliderComponent*> mStaticColliders;
    };
}
