# Particles

A particle effect is a `ParticleEffectsComponent` subclass. The component owns a fixed pool of particles, and
the subclass moves, spawns and kills them in `Emit`. Alive particles are drawn as camera-facing quads in one
instanced draw call.

## Writing an effect

```cpp
#include "SpinoCore/Core/Actors/Components/ParticleEffectsComponent.h"
#include "SpinoCore/Utils/Random.h"

namespace Components = SpinoCore::Core::Actors::Components;

class SparksEffect final : public Components::ParticleEffectsComponent {
public:
    SparksEffect(SpinoCore::Core::Actors::Actor& actor, const Components::ParticleEffectsInput& input)
        : ParticleEffectsComponent(actor, input), mAges(GetCapacity(), 0.0f) {}

protected:
    void Emit(const float deltaTime, const std::span<Components::Particle> particles) override {
        using SpinoCore::Utils::Random::GetFloatRange;
        for (std::size_t i = 0; i < particles.size(); ++i) {
            Components::Particle& particle = particles[i];
            if (!particle.alive) {
                particle.alive = true;
                particle.position = GetActor()->GetPosition();
                particle.color = {1.0f, 0.7f, 0.2f, 1.0f};
                particle.size = 0.1f;
                mAges[i] = GetFloatRange(0.0f, 0.5f);
                continue;
            }
            mAges[i] += deltaTime;
            particle.position.y += 2.0f * deltaTime;
            particle.color.w = 1.0f - mAges[i];
            if (mAges[i] >= 1.0f) particle.alive = false;
        }
    }

private:
    std::vector<float> mAges;
};
```

`Particle` has `position`, `size`, `color` (RGBA), `rotation` (radians, around the view axis) and `alive`. Keep
extra data, such as velocities and ages, in arrays indexed like the pool.

## Using it

```cpp
Components::ParticleEffectsInput input{};
input.capacity = 256;
input.blend = Components::ParticleBlend::Additive;

auto torch = std::make_unique<StaticActor>(SpinoCore::Core::Actors::ActorInput{.position = {1.0f, 0.0f, 0.0f}});
torch->AddComponent(std::make_unique<SparksEffect>(*torch, input));
ctx.actorsManager.AddActor(std::move(torch));
```

`ParticleBlend::Additive` brightens the scene (fire, sparks, magic). `ParticleBlend::Alpha` uses normal
transparency (smoke, dust). `GetAliveCount()` returns how many particles are alive.
