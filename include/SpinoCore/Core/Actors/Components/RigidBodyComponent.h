// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "Component.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::Actors::Components {
    /// @brief Settings of a rigid body.
    struct RigidBodyInput : ComponentInput {
        /// Whether gravity pulls the body down.
        bool applyGravity{true};
        /// Gravity acceleration along -Y.
        float gravity{10.0f};
        /// Mass; 0 makes the body immovable by impulses.
        float mass{1.0f};
        /// Speed limit on each axis.
        Math::Vector3f maxVelocity{Math::Vector3f{1000.0f, 1000.0f, 1000.0f}};
        /// Fraction of the velocity lost per second on each axis.
        Math::Vector3f frictionCoefficient{Math::Vector3f::Zero()};
        /// Bounciness from 0 to 1.
        float restitutionCoefficient{0.0f};
    };

    /// @brief Moves the actor with velocity, forces, gravity and friction.
    class RigidBodyComponent : public Component {
    public:
        /// @brief Creates a rigid body at rest.
        /// @param actor Owner; its position is moved on each update.
        /// @param input Body settings.
        explicit RigidBodyComponent(Actor& actor, const RigidBodyInput& input = {});
        ~RigidBodyComponent() override = default;

        /// @brief Integrates forces and velocity and moves the actor.
        /// @param deltaTime Simulation step in seconds.
        void Update(float deltaTime) override;
        /// @brief Adds a force for the next update only.
        /// @param force Force in world space.
        void ApplyForce(const Math::Vector3f &force);
        /// @brief Changes the velocity at once.
        /// @param impulse Impulse in world space; it is divided by the mass.
        void ApplyImpulse(const Math::Vector3f &impulse);

        /// @brief Replaces the velocity.
        /// @param velocity New velocity.
        void SetVelocity(const Math::Vector3f &velocity) { mVelocity = velocity; }
        /// @return One over the mass, or 0 for a massless body.
        [[nodiscard]] float GetInverseMass() const { return mMass > 0.0f ? 1.0f / mMass : 0.0f; }

        /// @return The velocity.
        [[nodiscard]] const Math::Vector3f& GetVelocity() const { return mVelocity; }
        /// @return The speed limit on each axis.
        [[nodiscard]] const Math::Vector3f& GetMaxVelocity() const { return mMaxVelocity; }
        /// @brief Changes the speed limit.
        /// @param maxVelocity New limit on each axis.
        void SetMaxVelocity(const Math::Vector3f &maxVelocity) { mMaxVelocity = maxVelocity; }

        /// @return Whether gravity is applied.
        [[nodiscard]] bool GetApplyGravity() const { return mApplyGravity; }
        /// @brief Turns gravity on or off.
        /// @param applyGravity true to apply gravity.
        void SetApplyGravity(const bool applyGravity) { mApplyGravity = applyGravity; }

        /// @return The gravity acceleration.
        [[nodiscard]] float GetGravity() const { return mGravity; }
        /// @brief Changes the gravity acceleration.
        /// @param gravity New acceleration along -Y.
        void SetGravity(const float gravity) { mGravity = gravity; }

        /// @return The mass.
        [[nodiscard]] float GetMass() const { return mMass; }
        /// @brief Changes the mass.
        /// @param mass New mass.
        void SetMass(const float mass) { mMass = mass; }

        /// @return The friction on each axis.
        [[nodiscard]] const Math::Vector3f& GetFrictionCoefficient() const { return mFrictionCoefficient; }
        /// @brief Changes the friction.
        /// @param friction Fraction of the velocity lost per second on each axis.
        void SetFrictionCoefficient(const Math::Vector3f &friction) { mFrictionCoefficient = friction; }

        /// @return The bounciness.
        [[nodiscard]] float GetRestitutionCoefficient() const { return mRestitutionCoefficient; }
        /// @brief Changes the bounciness.
        /// @param coefficient Value from 0 to 1.
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
