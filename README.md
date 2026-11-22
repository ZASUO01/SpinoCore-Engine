# SpinoCore

SpinoCore is a small 3D game engine written in C++20 on top of SDL3 and OpenGL 4.6. A game is a set of
scenes; each scene creates actors with components (models, animation, colliders, rigid bodies, particles), UI
screens, cameras, lights, post-processing effects, sound clips and synthesizer music.

Features:

- fixed-step game loop (60 Hz) and a stack of scenes;
- asset manifest with incremental loading of textures, fonts, glTF models and sounds;
- actors with a transform hierarchy and components;
- glTF 2.0 models with materials, textures and skeletal animation;
- directional light with shadow mapping, and full-screen post-processing;
- sphere, capsule, box and plane colliders with rigid bodies and bounce;
- GPU-instanced particle effects;
- UI with text, images, buttons, input fields and selects;
- sound clips on 16 slots and a subtractive synthesizer;
- logging with `std::format` syntax.

## Requirements

- A C++20 compiler with `<format>` support (developed with GCC 14).
- CMake 3.24 or newer.
- A GPU and driver with OpenGL 4.6.

SDL3, SDL3_image, SDL3_ttf and SDL3_mixer are downloaded by CMake. glad, cgltf and nlohmann/json are in
`vendor/`.

## Build

The engine is a static library, `SpinoCore::Runtime`, meant to be added to a game project:

```
MyGame/
├── Assets/
│   └── manifest.json
├── Engine/            (this repository)
├── Source/
│   └── Main.cpp
└── CMakeLists.txt
```

```cmake
cmake_minimum_required(VERSION 3.24)
project(MyGame LANGUAGES C CXX)

add_subdirectory(Engine)

add_executable(MyGame Source/Main.cpp)
target_compile_features(MyGame PRIVATE cxx_std_20)
target_link_libraries(MyGame PRIVATE SpinoCore::Runtime)
spinocore_add_assets(MyGame)
```

`spinocore_add_assets` copies the `Assets` folder next to the executable on every build.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Options:

| Option | Default | Effect |
|---|---|---|
| `SPINOCORE_BUILD_TESTS` | `OFF` | Builds the unit, integration and end-to-end tests. |
| `SPINOCORE_WARNINGS_AS_ERRORS` | `OFF` | Treats compiler warnings as errors in the engine. |

### Tests

```bash
cmake -S . -B build-test -DSPINOCORE_BUILD_TESTS=ON
cmake --build build-test
ctest --test-dir build-test/Engine
```

The integration tests need a display with OpenGL 4.6.

## Quickstart

`Source/Main.cpp`:

```cpp
#include <memory>
#include "SpinoCore/Core/Actors/Actor.h"
#include "SpinoCore/Core/Actors/ActorsManager.h"
#include "SpinoCore/Core/Actors/Components/ModelComponent.h"
#include "SpinoCore/Core/Rendering/BuiltinModels.h"
#include "SpinoCore/Core/Scenes/Contexts.h"
#include "SpinoCore/Core/Scenes/Scene.h"
#include "SpinoCore/Core/Scenes/ScenesManager.h"
#include "SpinoCore/Files/FileManager.h"
#include "SpinoCore/Game/GameApplication.h"
#include "SpinoCore/Game/InitializationContext.h"
#include "SpinoCore/Launch.h"
#include "SpinoCore/LaunchOptions.h"

namespace Scenes = SpinoCore::Core::Scenes;
namespace Actors = SpinoCore::Core::Actors;

class Spinner final : public Actors::Actor {
public:
    using Actor::Actor;

protected:
    void OnUpdate(const float deltaTime) override {
        SetRotation(GetRotation() * SpinoCore::Math::Quaternionf{SpinoCore::Math::Vector3f::UnitY(), deltaTime});
    }

    void OnProcessInput(const SpinoCore::Core::Input::InputState&) override {}
};

class HelloScene final : public Scenes::Scene {
public:
    void OnStart(const Scenes::StartContext& ctx) override {
        ctx.fileManager.LoadFromManifest("manifest.json");
    }

    void OnSetup(const Scenes::ControlContext& ctx) override {
        auto cube = std::make_unique<Spinner>();
        cube->AddComponent(std::make_unique<Actors::Components::ModelComponent>(
            *cube, std::string(SpinoCore::Core::Rendering::CubeModelId)));
        ctx.actorsManager.AddActor(std::move(cube));
    }

    void OnUpdate(const Scenes::ControlContext&) override {}
    void OnFinish(const Scenes::ControlContext&) override {}
};

class HelloGame final : public SpinoCore::Game::GameApplication {
public:
    void Initialize(const SpinoCore::Game::InitializationContext& ctx) override {
        ctx.scenesManager.PushScene(std::make_unique<HelloScene>());
    }
};

int main() {
    SpinoCore::LaunchOptions options{};
    options.windowProps.title = "Hello SpinoCore";
    options.enableGlobalLight = true;
    return SpinoCore::Launch(std::make_unique<HelloGame>(), options);
}
```

`Assets/manifest.json`:

```json
{ "version": "1.0", "assets": [] }
```

## Documentation

Tutorials:

1. [Launch and game](docs/tutorials/01-launch-and-game.md)
2. [Scenes and contexts](docs/tutorials/02-scenes-and-contexts.md)
3. [Files and manifest](docs/tutorials/03-files-and-manifest.md)
4. [UI](docs/tutorials/04-ui.md)
5. [Input](docs/tutorials/05-input.md)
6. [Cameras](docs/tutorials/06-cameras.md)
7. [Actors and components](docs/tutorials/07-actors-and-components.md)
8. [Models and animation](docs/tutorials/08-models-and-animation.md)
9. [Lighting and shadows](docs/tutorials/09-lighting-and-shadows.md)
10. [Post-processing](docs/tutorials/10-post-processing.md)
11. [Collisions and physics](docs/tutorials/11-collisions-and-physics.md)
12. [Particles](docs/tutorials/12-particles.md)
13. [Synth](docs/tutorials/13-synth.md)
14. [Sounds](docs/tutorials/14-sounds.md)
15. [Logs](docs/tutorials/15-logs.md)
16. [Math and utils](docs/tutorials/16-math-and-utils.md)

The API reference is generated with Doxygen from the headers in `include/SpinoCore`:

```bash
doxygen Doxyfile
```

Run it in the folder that contains `Engine/`; the HTML pages are written to `docs/api/html`.

## License

MIT. See [LICENSE](LICENSE).
