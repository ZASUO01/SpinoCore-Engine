// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <functional>
#include <utility>
#include "Component.h"

namespace SpinoCore::Physics {
    class CollisionProcessor;
}

namespace SpinoCore::Core::Actors::Components {
    /// @brief Shape of a collider.
    enum class ColliderType {
        Sphere,
        Capsule,
        Box,
        Plane
    };

    /// @brief Initial settings shared by every collider.
    struct ColliderInput : ComponentInput {
        /// Shape; each collider component sets its own.
        ColliderType type{ColliderType::Box};
        /// Static colliders never move when they collide.
        bool isStatic{true};
        /// Triggers report collisions but do not push anything.
        bool isTrigger{false};
        /// Bounciness from 0 (no bounce) to 1 (full bounce).
        float restitutionCoefficient{0.0f};
        /// Speed below which a collision does not bounce.
        float minRestitutionVelocity{15.0f};
    };

    /// @brief Base class of the colliders checked by the collision system.
    class ColliderComponent : public Component {
    public:
        /// @brief Function called with the other collider when a collision happens.
        using CollisionCallback = std::function<void(ColliderComponent& other)>;

        /// @brief Creates a collider.
        /// @param actor Owner of the collider.
        /// @param input Initial settings.
        explicit ColliderComponent(Actor& actor, const ColliderInput& input = {})
        :Component(actor, input)
        ,mType(input.type)
        ,mIsStatic(input.isStatic)
        ,mIsTrigger(input.isTrigger)
        ,mRestitutionCoefficient(input.restitutionCoefficient)
        ,mMinRestitutionVelocity(input.minRestitutionVelocity){}

        ~ColliderComponent() override = default;

        /// @return Whether the collider never moves on collisions.
        [[nodiscard]] bool IsStatic() const { return mIsStatic;}
        /// @return Whether the collider only reports collisions.
        [[nodiscard]] bool IsTrigger() const { return mIsTrigger; }
        /// @return The shape of the collider.
        [[nodiscard]] ColliderType GetType() const { return mType; }
        /// @return Whether the collider touched another one in the last step.
        [[nodiscard]] bool IsColliding() const { return mIsColliding; }

        /// @brief Calls the collision callback, if any.
        /// @param other Collider that was hit.
        void OnCollisionCallBack(ColliderComponent& other) const { if (mCollisionCallBack) { mCollisionCallBack(other); } }
        /// @brief Sets the function called on each collision.
        /// @param callback Function that receives the other collider.
        void SetCollisionCallBack(CollisionCallback callback) { mCollisionCallBack = std::move(callback); }

        /// @brief Changes the bounciness.
        /// @param restitutionCoefficient Value from 0 to 1.
        void SetRestitution(const float restitutionCoefficient) { mRestitutionCoefficient = restitutionCoefficient; }
        /// @return The bounciness.
        [[nodiscard]] float GetRestitution() const { return mRestitutionCoefficient; }

        /// @brief Changes the speed below which collisions do not bounce.
        /// @param restitutionVelocity Minimum speed.
        void SetMinRestitutionVelocity(const float restitutionVelocity) { mMinRestitutionVelocity = restitutionVelocity; }
        /// @return The minimum speed for a bounce.
        [[nodiscard]] float GetMinRestitutionVelocity() const { return mMinRestitutionVelocity; }

    private:
        friend class Physics::CollisionProcessor;

        ColliderType mType;
        bool mIsStatic{true};
        bool mIsTrigger{false};
        bool mIsColliding{false};

        float mRestitutionCoefficient{0.0f};
        float mMinRestitutionVelocity{15.0f};

        CollisionCallback mCollisionCallBack{nullptr};
    };
}
