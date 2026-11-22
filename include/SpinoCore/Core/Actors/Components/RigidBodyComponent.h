// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "Component.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::Actors::Components {
    struct RigidBodyInput : ComponentInput {
        bool applyGravity{true};
        float gravity{10.0f};
        float mass{1.0f};
        Math::Vector3f maxVelocity{Math::Vector3f{1000.0f, 1000.0f, 1000.0f}};
        Math::Vector3f frictionCoefficient{Math::Vector3f::Zero()};
        float restitutionCoefficient{0.0f};
    };

    class RigidBodyComponent : public Component {
    public:
        explicit RigidBodyComponent(Actor& actor, const RigidBodyInput& input = {});
        ~RigidBodyComponent() override = default;

        void Update(float deltaTime) override;
        void ApplyForce(const Math::Vector3f &force);

        [[nodiscard]] const Math::Vector3f& GetVelocity() const { return mVelocity; }
        [[nodiscard]] const Math::Vector3f& GetMaxVelocity() const { return mMaxVelocity; }
        void SetMaxVelocity(const Math::Vector3f &maxVelocity) { mMaxVelocity = maxVelocity; }

        [[nodiscard]] bool GetApplyGravity() const { return mApplyGravity; }
        void SetApplyGravity(const bool applyGravity) { mApplyGravity = applyGravity; }

        [[nodiscard]] float GetGravity() const { return mGravity; }
        void SetGravity(const float gravity) { mGravity = gravity; }

        [[nodiscard]] float GetMass() const { return mMass; }
        void SetMass(const float mass) { mMass = mass; }

        [[nodiscard]] const Math::Vector3f& GetFrictionCoefficient() const { return mFrictionCoefficient; }
        void SetFrictionCoefficient(const Math::Vector3f &friction) { mFrictionCoefficient = friction; }

        [[nodiscard]] float GetRestitutionCoefficient() const { return mRestitutionCoefficient; }
        void SetRestitutionCoefficient(const float coefficient) { mRestitutionCoefficient = coefficient; }
    private:
        bool mApplyGravity;
        float mGravity{10.0f};
        float mMass{1.0f};
        Math::Vector3f mFrictionCoefficient{Math::Vector3f::Zero()};
        float mRestitutionCoefficient{0.0f};

        Math::Vector3f mVelocity{Math::Vector3f::Zero()};
        Math::Vector3f mMaxVelocity{Math::Vector3f{1000.0f, 1000.0f, 1000.0f}};
        Math::Vector3f mAcceleration{Math::Vector3f::Zero()};
    };
}
