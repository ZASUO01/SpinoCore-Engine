// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/Actors/Components/ParticleEffectsComponent.h"
#include <algorithm>
#include "Core/Rendering/Drawer/DrawCommands.h"

namespace SpinoCore::Core::Actors::Components {
    ParticleEffectsComponent::ParticleEffectsComponent(Actor& actor, const ParticleEffectsInput& input)
    :Component(actor, input)
    ,mParticles(input.capacity)
    ,mBlend(input.blend)
    {}

    void ParticleEffectsComponent::Update(const float deltaTime) {
        Emit(deltaTime, mParticles);
    }

    std::size_t ParticleEffectsComponent::GetAliveCount() const {
        return static_cast<std::size_t>(std::ranges::count_if(mParticles, &Particle::alive));
    }

    void ParticleEffectsComponent::GetParticleDrawCommand(Rendering::ParticleDrawList& drawList) {
        const std::size_t first = drawList.instances.size();
        for (const Particle& particle : mParticles) {
            if (!particle.alive) continue;
            drawList.instances.push_back(Rendering::ParticleInstance{
                .position = particle.position,
                .size = particle.size,
                .color = particle.color,
                .rotation = particle.rotation,
            });
        }

        const std::size_t count = drawList.instances.size() - first;
        if (count > 0) drawList.batches.push_back(Rendering::ParticleBatch{.first = first, .count = count, .additive = mBlend == ParticleBlend::Additive});
    }
}
