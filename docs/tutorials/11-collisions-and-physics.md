# Collisions and physics

Collisions are detected between collider components after the actors update. Non-static colliders are pushed
out of what they hit; when their actor has a `RigidBodyComponent`, the push is weighted by mass and the body
also bounces.

## Colliders

| Component | Shape | Input fields |
|---|---|---|
| `SphereColliderComponent` | Sphere | `radius`, `offset` |
| `CapsuleColliderComponent` | Vertical capsule | `radius`, `halfHeight` |
| `BoxColliderComponent` | Oriented box | `extent` (half size) |
| `PlaneColliderComponent` | Infinite static plane | `normal`, `distanceToOrigin` |

Every collider input also has:

- `isStatic`: static colliders are never moved by collisions;
- `isTrigger`: triggers report collisions but push nothing;
- `restitutionCoefficient`: bounciness from 0 to 1;
- `minRestitutionVelocity`: speed below which there is no bounce.

Colliders follow the actor transform, including its scale.

## A falling ball

```cpp
#include "SpinoCore/Core/Actors/Components/PlaneColliderComponent.h"
#include "SpinoCore/Core/Actors/Components/RigidBodyComponent.h"
#include "SpinoCore/Core/Actors/Components/SphereColliderComponent.h"

namespace Components = SpinoCore::Core::Actors::Components;

auto ground = std::make_unique<StaticActor>(SpinoCore::Core::Actors::ActorInput{});
ground->AddComponent(std::make_unique<Components::PlaneColliderComponent>(*ground, Components::PlaneColliderInput{}));
ctx.actorsManager.AddActor(std::move(ground));

auto ball = std::make_unique<StaticActor>(SpinoCore::Core::Actors::ActorInput{.position = {0.0f, 4.0f, 0.0f}});
Components::SphereColliderInput sphere{};
sphere.radius = 0.35f;
sphere.isStatic = false;
sphere.restitutionCoefficient = 0.5f;
ball->AddComponent(std::make_unique<Components::SphereColliderComponent>(*ball, sphere));
ball->AddComponent(std::make_unique<Components::RigidBodyComponent>(*ball));
ctx.actorsManager.AddActor(std::move(ball));
```

## Collision callbacks

```cpp
auto* trigger = coin->GetComponent<Components::SphereColliderComponent>();
trigger->SetCollisionCallBack([coin](Components::ColliderComponent& other) {
    if (other.GetActor()->GetComponent<PlayerTag>()) coin->SetState(SpinoCore::Core::Actors::Actor::State::Destroy);
});
```

`IsColliding()` tells if the collider touched another one in the last step.

## Rigid bodies

`RigidBodyInput` sets `mass`, `applyGravity`, `gravity` (along -Y), `maxVelocity`, `frictionCoefficient`
(velocity lost per second on each axis) and `restitutionCoefficient`.

| Call | Effect |
|---|---|
| `ApplyForce(force)` | Acceleration for the next step only. |
| `ApplyImpulse(impulse)` | Immediate change of velocity, divided by the mass. |
| `SetVelocity(velocity)` | Replaces the velocity. |

## Raycasts

`ActorsManager::Raycast` returns the nearest enabled collider hit by a ray, or nothing. Triggers and static
colliders are hit too; the optional filter skips colliders.

```cpp
#include "SpinoCore/Physics/Ray.h"

const SpinoCore::Physics::Ray ray{.origin = eye, .direction = forward};
const auto hit = ctx.actorsManager.Raycast(ray, 100.0f, [](const Components::ColliderComponent& collider) {
    return collider.GetActor()->GetComponent<EnemyTag>() != nullptr;
});
if (hit) Logger::Info("Hit at {} meters", hit->distance);
```

`RaycastHit` has the `collider`, the `point`, the surface `normal` and the `distance` along the ray. The direction
does not need to be normalized; a zero direction hits nothing.

## Debugging

`LaunchOptions::debugColliders = true` draws the outline of every collider.
