# Actors and components

Actors are the objects of a scene. An actor has a transform, an optional parent and a list of components that
give it behaviour: models, animation, physics, colliders and particles.

## An actor

```cpp
#include "SpinoCore/Core/Actors/Actor.h"

class Spinner final : public SpinoCore::Core::Actors::Actor {
public:
    explicit Spinner(const SpinoCore::Core::Actors::ActorInput& input) : Actor(input) {}

protected:
    void OnUpdate(const float deltaTime) override {
        using namespace SpinoCore::Math;
        SetRotation(GetRotation() * Quaternionf{Vector3f::UnitY(), 1.5f * deltaTime});
    }

    void OnProcessInput(const SpinoCore::Core::Input::InputState&) override {}
};
```

`ActorInput` sets the initial `position`, `rotation`, `scale` and `anchor`. The anchor moves the actor without
moving its rotation pivot.

## Adding actors and components

```cpp
#include "SpinoCore/Core/Actors/ActorsManager.h"
#include "SpinoCore/Core/Actors/Components/ModelComponent.h"
#include "SpinoCore/Core/Rendering/BuiltinModels.h"

void OnSetup(const ControlContext& ctx) override {
    auto crate = std::make_unique<Spinner>(SpinoCore::Core::Actors::ActorInput{.position = {0.0f, 0.5f, 0.0f}});
    crate->AddComponent(std::make_unique<SpinoCore::Core::Actors::Components::ModelComponent>(
        *crate, std::string(SpinoCore::Core::Rendering::CubeModelId)));
    SpinoCore::Core::Actors::Actor* added = ctx.actorsManager.AddActor(std::move(crate));
}
```

`AddActor` returns the actor so the scene can keep a pointer to it. The manager owns it and destroys it when the
scene changes.

## Update order and state

- Actors update by `SetUpdateOrder` (lower first, default 100); components inside an actor do the same.
- `SetState(State::Paused)` keeps the actor drawn but stops updates and input.
- `SetState(State::Destroy)` removes the actor after the current update.
- Actors and components added during an update start on the next update.

## Hierarchy

```cpp
weapon->SetParent(player);
```

A child keeps its local transform and follows the parent. `GetWorldTransform()` returns the combined
transform, and `GetForward()` the direction the actor faces (local -Z).

## Writing a component

```cpp
#include "SpinoCore/Core/Actors/Components/Component.h"

class Bobbing final : public SpinoCore::Core::Actors::Components::Component {
public:
    using Component::Component;

    void Update(const float deltaTime) override {
        mTime += deltaTime;
        auto* actor = GetActor();
        auto position = actor->GetPosition();
        position.y = 0.5f + 0.2f * SpinoCore::Math::Operations::Sin(mTime * 3.0f);
        actor->SetPosition(position);
    }

private:
    float mTime{0.0f};
};
```

`GetComponent<T>()` finds a component by type, and `RemoveComponent` removes one.
