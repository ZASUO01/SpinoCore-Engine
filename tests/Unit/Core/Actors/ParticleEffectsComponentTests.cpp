// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <cstddef>
#include <memory>
#include <span>
#include <utility>
#include <gtest/gtest.h>
#include "Core/Rendering/Drawer/DrawCommands.h"
#include "SpinoCore/Core/Actors/Actor.h"
#include "SpinoCore/Core/Actors/Components/ParticleEffectsComponent.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Core::Actors::Actor;
    using SpinoCore::Core::Actors::ActorInput;
    using SpinoCore::Core::Actors::Components::Particle;
    using SpinoCore::Core::Actors::Components::ParticleBlend;
    using SpinoCore::Core::Actors::Components::ParticleEffectsComponent;
    using SpinoCore::Core::Actors::Components::ParticleEffectsInput;
    using SpinoCore::Core::Rendering::ParticleDrawList;
    using SpinoCore::Math::Vector3f;

    class StillActor final : public Actor {
    public:
        StillActor() : Actor(ActorInput{}) {}

    protected:
        void OnUpdate(float) override {}
        void OnProcessInput(const SpinoCore::Core::Input::InputState&) override {}
    };

    class OneParticlePerTick final : public ParticleEffectsComponent {
    public:
        OneParticlePerTick(Actor& actor, const ParticleEffectsInput& input) : ParticleEffectsComponent(actor, input) {}

        int emits{0};
        float lastDeltaTime{0.0f};

    protected:
        void Emit(const float deltaTime, const std::span<Particle> particles) override {
            ++emits;
            lastDeltaTime = deltaTime;
            for (Particle& particle : particles) {
                if (particle.alive) continue;
                particle.alive = true;
                particle.position = Vector3f{static_cast<float>(emits), 0.0f, 0.0f};
                return;
            }
        }
    };

    [[nodiscard]] ParticleEffectsInput Capacity(const std::size_t capacity, const ParticleBlend blend = ParticleBlend::Additive) {
        ParticleEffectsInput input{};
        input.capacity = capacity;
        input.blend = blend;
        return input;
    }

    TEST(ParticleEffectsComponentTests, TheBufferIsAllocatedWithItsCapacity) {
        StillActor actor;

        const OneParticlePerTick effect{actor, Capacity(8)};

        EXPECT_EQ(effect.GetCapacity(), 8u);
    }

    TEST(ParticleEffectsComponentTests, TheBufferStartsWithNoLiveParticles) {
        StillActor actor;

        const OneParticlePerTick effect{actor, Capacity(8)};

        EXPECT_EQ(effect.GetAliveCount(), 0u);
    }

    TEST(ParticleEffectsComponentTests, EmitIsCalledOncePerTick) {
        StillActor actor;
        OneParticlePerTick effect{actor, Capacity(8)};

        effect.Update(0.016f);
        effect.Update(0.016f);
        effect.Update(0.016f);

        EXPECT_EQ(effect.emits, 3);
    }

    TEST(ParticleEffectsComponentTests, EmitReceivesTheTickDuration) {
        StillActor actor;
        OneParticlePerTick effect{actor, Capacity(8)};

        effect.Update(0.25f);

        EXPECT_FLOAT_EQ(effect.lastDeltaTime, 0.25f);
    }

    TEST(ParticleEffectsComponentTests, TheBufferNeverGrows) {
        StillActor actor;
        OneParticlePerTick effect{actor, Capacity(4)};

        for (int tick = 0; tick < 20; ++tick) effect.Update(0.016f);

        EXPECT_EQ(effect.GetParticles().size(), 4u);
    }

    TEST(ParticleEffectsComponentTests, OnlyLiveParticlesAreDrawn) {
        StillActor actor;
        OneParticlePerTick effect{actor, Capacity(8)};
        effect.Update(0.016f);
        effect.Update(0.016f);
        ParticleDrawList drawList{};

        effect.GetParticleDrawCommand(drawList);

        EXPECT_EQ(drawList.instances.size(), 2u);
    }

    TEST(ParticleEffectsComponentTests, AnEffectWithoutLiveParticlesAddsNoBatch) {
        StillActor actor;
        OneParticlePerTick effect{actor, Capacity(8)};
        ParticleDrawList drawList{};

        effect.GetParticleDrawCommand(drawList);

        EXPECT_TRUE(drawList.batches.empty());
    }

    TEST(ParticleEffectsComponentTests, TheBatchKeepsTheBlendOfTheEffect) {
        StillActor actor;
        OneParticlePerTick effect{actor, Capacity(8, ParticleBlend::Alpha)};
        effect.Update(0.016f);
        ParticleDrawList drawList{};

        effect.GetParticleDrawCommand(drawList);

        ASSERT_EQ(drawList.batches.size(), 1u);
        EXPECT_FALSE(drawList.batches.front().additive);
    }

    TEST(ParticleEffectsComponentTests, ADisabledEffectDoesNotEmit) {
        StillActor actor;
        auto owned = std::make_unique<OneParticlePerTick>(actor, Capacity(8));
        OneParticlePerTick& effect = *owned;
        actor.AddComponent(std::move(owned));
        effect.SetEnabled(false);

        actor.Update(0.016f);

        EXPECT_EQ(effect.emits, 0);
    }
}
