// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>
#include "Component.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::Rendering {
    struct ParticleDrawList;
}

namespace SpinoCore::Core::Actors::Components {
    /// @brief One particle drawn as a camera-facing quad.
    struct Particle {
        /// Position in world space.
        Math::Vector3f position{Math::Vector3f::Zero()};
        /// Width and height of the quad.
        float size{0.1f};
        /// RGBA color multiplied into the particle.
        Math::Vector4f color{Math::Vector4f::One()};
        /// Rotation of the quad around the view axis, in radians.
        float rotation{0.0f};
        /// Only alive particles are drawn.
        bool alive{false};
    };

    /// @brief How particles are blended with the scene.
    enum class ParticleBlend : std::uint8_t {
        /// Standard transparency.
        Alpha,
        /// Colors are added, which brightens the scene.
        Additive
    };

    /// @brief Settings of a particle effect.
    struct ParticleEffectsInput : ComponentInput {
        /// Number of particles in the pool.
        std::size_t capacity{256};
        /// Blend mode.
        ParticleBlend blend{ParticleBlend::Additive};
    };

    /// @brief Base class of particle effects; subclasses implement Emit to move and spawn particles.
    class ParticleEffectsComponent : public Component {
    public:
        /// @brief Creates an effect with every particle dead.
        /// @param actor Owner of the effect.
        /// @param input Pool size and blend mode.
        explicit ParticleEffectsComponent(Actor& actor, const ParticleEffectsInput& input = {});

        /// @brief Calls Emit with the particle pool.
        /// @param deltaTime Simulation step in seconds.
        void Update(float deltaTime) final;
        /// @brief Adds the alive particles to the draw list.
        /// @param drawList List that receives the particles.
        void GetParticleDrawCommand(Rendering::ParticleDrawList& drawList) final;

        /// @return The whole particle pool.
        [[nodiscard]] std::span<const Particle> GetParticles() const { return mParticles; }
        /// @return The size of the pool.
        [[nodiscard]] std::size_t GetCapacity() const { return mParticles.size(); }
        /// @return The number of alive particles.
        [[nodiscard]] std::size_t GetAliveCount() const;
        /// @return The blend mode.
        [[nodiscard]] ParticleBlend GetBlend() const { return mBlend; }

    protected:
        virtual void Emit(float deltaTime, std::span<Particle> particles) = 0;

    private:
        std::vector<Particle> mParticles;
        ParticleBlend mBlend;
    };
}
