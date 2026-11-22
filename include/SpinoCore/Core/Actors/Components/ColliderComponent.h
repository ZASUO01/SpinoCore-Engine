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
    enum class ColliderType {
        Sphere,
        Capsule,
        Box,
        Plane
    };

    struct ColliderInput : ComponentInput {
        ColliderType type{ColliderType::Box};
        bool isStatic{true};
        bool isTrigger{false};
        float restitutionCoefficient{0.0f};
        float minRestitutionVelocity{15.0f};
    };

    class ColliderComponent : public Component {
    public:
        using CollisionCallback = std::function<void(ColliderComponent& other)>;

        explicit ColliderComponent(Actor& actor, const ColliderInput& input = {})
        :Component(actor, input)
        ,mType(input.type)
        ,mIsStatic(input.isStatic)
        ,mIsTrigger(input.isTrigger)
        ,mRestitutionCoefficient(input.restitutionCoefficient)
        ,mMinRestitutionVelocity(input.minRestitutionVelocity){}

        ~ColliderComponent() override = default;

        [[nodiscard]] bool IsStatic() const { return mIsStatic;}
        [[nodiscard]] bool IsTrigger() const { return mIsTrigger; }
        [[nodiscard]] ColliderType GetType() const { return mType; }
        [[nodiscard]] bool IsColliding() const { return mIsColliding; }

        void OnCollisionCallBack(ColliderComponent& other) const { if (mCollisionCallBack) { mCollisionCallBack(other); } }
        void SetCollisionCallBack(CollisionCallback callback) { mCollisionCallBack = std::move(callback); }

        void SetRestitution(const float restitutionCoefficient) { mRestitutionCoefficient = restitutionCoefficient; }
        [[nodiscard]] float GetRestitution() const { return mRestitutionCoefficient; }

        void SetMinRestitutionVelocity(const float restitutionVelocity) { mMinRestitutionVelocity = restitutionVelocity; }
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
