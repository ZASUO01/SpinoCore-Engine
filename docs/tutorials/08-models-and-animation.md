# Models and animation

Models are glTF 2.0 files (`.gltf` or `.glb`) loaded into the model cache. They can have several meshes, base
color textures and materials, a skeleton and animation clips.

## Loading

```json
{ "name": "character", "type": "Model", "path": "Models/character.glb" }
```

Textures referenced by the glTF are loaded relative to the model; embedded images are cached as
`<model name>#<index>`. The manifest can override the texture of every material with `"texture"` or point to a
material file with `"material"` (see [Files and manifest](03-files-and-manifest.md)).

The cube `SpinoCore::Core::Rendering::CubeModelId` is always available, and a model that fails to load is drawn
as that cube.

## Drawing

```cpp
#include "SpinoCore/Core/Actors/Components/ModelComponent.h"

auto hero = std::make_unique<HeroActor>(SpinoCore::Core::Actors::ActorInput{.position = {0.0f, 0.0f, 0.0f}});
hero->AddComponent(std::make_unique<SpinoCore::Core::Actors::Components::ModelComponent>(*hero, "character"));
```

The model is drawn with the actor world transform. `SetModelId` swaps it at any time.

## Animation

`AnimatorComponent` plays the clips of the `ModelComponent` of the same actor.

```cpp
#include "SpinoCore/Core/Actors/Components/AnimatorComponent.h"

auto animator = std::make_unique<SpinoCore::Core::Actors::Components::AnimatorComponent>(*hero);
animator->Play("Walk");
hero->AddComponent(std::move(animator));
```

| Call | Effect |
|---|---|
| `Play(name, loop)` | Starts a clip from the beginning; `loop` defaults to true. |
| `Stop()` | Freezes the current pose. |
| `IsPlaying`, `GetCurrentClip`, `GetTime` | Read the playback state. |

A clip name that the model does not have is logged and the bind pose is shown. Keyframes use step or linear
interpolation (cubic spline clips are played linearly between their keys), and up to 64 bones per model are
sent to the GPU.
