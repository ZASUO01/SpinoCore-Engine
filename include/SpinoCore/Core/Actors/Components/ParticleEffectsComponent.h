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
    struct Particle {
        Math::Vector3f position{Math::Vector3f::Zero()};
        float size{0.1f};
        Math::Vector4f color{Math::Vector4f::One()};
        float rotation{0.0f};
        bool alive{false};
    };

    enum class ParticleBlend : std::uint8_t {
        Alpha,
        Additive
    };

    struct ParticleEffectsInput : ComponentInput {
        std::size_t capacity{256};
        ParticleBlend blend{ParticleBlend::Additive};
    };

    class ParticleEffectsComponent : public Component {
    public:
        explicit ParticleEffectsComponent(Actor& actor, const ParticleEffectsInput& input = {});

        void Update(float deltaTime) final;
        void GetParticleDrawCommand(Rendering::ParticleDrawList& drawList) final;

        [[nodiscard]] std::span<const Particle> GetParticles() const { return mParticles; }
        [[nodiscard]] std::size_t GetCapacity() const { return mParticles.size(); }
        [[nodiscard]] std::size_t GetAliveCount() const;
        [[nodiscard]] ParticleBlend GetBlend() const { return mBlend; }

    protected:
        virtual void Emit(float deltaTime, std::span<Particle> particles) = 0;

    private:
        std::vector<Particle> mParticles;
        ParticleBlend mBlend;
    };
}
