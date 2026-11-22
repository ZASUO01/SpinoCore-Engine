// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/Actors/Components/RigidBodyComponent.h"
#include <algorithm>
#include "SpinoCore/Core/Actors/Actor.h"
#include "SpinoCore/Math/Math.h"

namespace SpinoCore::Core::Actors::Components {
    RigidBodyComponent::RigidBodyComponent(Actor &actor, const RigidBodyInput &input)
    :Component(actor, input)
    ,mApplyGravity(input.applyGravity)
    ,mGravity(input.gravity)
    ,mMass(input.mass)
    ,mFrictionCoefficient(input.frictionCoefficient)
    ,mRestitutionCoefficient(input.restitutionCoefficient)
    ,mMaxVelocity(input.maxVelocity)
    {}

    void RigidBodyComponent::ApplyForce(const Math::Vector3f &force) {
        mAcceleration += force * (1.0f / mMass);
    }

    void RigidBodyComponent::ApplyImpulse(const Math::Vector3f &impulse) {
        mVelocity += impulse * GetInverseMass();
    }

    void RigidBodyComponent::Update(const float deltaTime) {
        if (mApplyGravity) {
            const Math::Vector3f gravityForce = Math::Vector3f::UnitY() * (-mGravity * mMass);
            ApplyForce(gravityForce);
        }

        mVelocity.x *= std::clamp(1.0f - mFrictionCoefficient.x * deltaTime, 0.0f, 1.0f);
        mVelocity.y *= std::clamp(1.0f - mFrictionCoefficient.y * deltaTime, 0.0f, 1.0f);
        mVelocity.z *= std::clamp(1.0f - mFrictionCoefficient.z * deltaTime, 0.0f, 1.0f);

        mVelocity += mAcceleration * deltaTime;

        mVelocity.x =  std::clamp(mVelocity.x, -mMaxVelocity.x, mMaxVelocity.x);
        mVelocity.y = std::clamp(mVelocity.y, -mMaxVelocity.y, mMaxVelocity.y);
        mVelocity.z = std::clamp(mVelocity.z, -mMaxVelocity.z, mMaxVelocity.z);

        if (Math::Operations::NearlyZero(mVelocity.x)) {
            mVelocity.x = 0.0f;
        }

        if (Math::Operations::NearlyZero(mVelocity.y)) {
            mVelocity.y = 0.0f;
        }

        if (Math::Operations::NearlyZero(mVelocity.z)) {
            mVelocity.z = 0.0f;
        }

        const auto actor = GetActor();
        actor->SetPosition(actor->GetPosition() + mVelocity * deltaTime);
        mAcceleration = Math::Vector3f::Zero();
    }
}

