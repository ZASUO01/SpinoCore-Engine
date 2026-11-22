# Scenes and contexts

A scene is a level, a menu or any other state of the game. Scenes are stacked by the `ScenesManager` and
only the top one runs.

## Lifecycle

| Hook | When | Context |
|---|---|---|
| `OnStart` | The scene is pushed. | `StartContext` |
| `OnSetup` | Every requested asset has finished loading. | `ControlContext` |
| `OnUpdate` | Every simulation step (60 per second). | `ControlContext` |
| `OnFinish` | The scene is replaced, popped or destroyed. | `ControlContext` |

`OnStart` should only request assets. Everything that uses them (screens, actors, models, sounds) belongs in
`OnSetup`.

```cpp
#include "SpinoCore/Core/Scenes/Contexts.h"
#include "SpinoCore/Core/Scenes/Scene.h"
#include "SpinoCore/Core/UI/ScreensManager.h"
#include "SpinoCore/Files/FileManager.h"

class MenuScene final : public SpinoCore::Core::Scenes::Scene {
public:
    void OnStart(const SpinoCore::Core::Scenes::StartContext& ctx) override {
        ctx.fileManager.LoadFromManifest("manifest.json");
    }

    void OnSetup(const SpinoCore::Core::Scenes::ControlContext& ctx) override {
        ctx.screensManager.PushScreen(std::make_unique<MenuScreen>());
    }

    void OnUpdate(const SpinoCore::Core::Scenes::ControlContext&) override {}
    void OnFinish(const SpinoCore::Core::Scenes::ControlContext&) override {}
};
```

## Contexts

`StartContext` only has the `fileManager`. `ControlContext` gives access to every module:

| Field | Module |
|---|---|
| `fileManager` | Read-only file manager (failed loads, file checks). |
| `camerasManager` | Cameras. |
| `screensManager` | UI screens. |
| `windowManager` | Window mode and size. |
| `actorsManager` | Actors. |
| `globalLight` | Light and shadows. |
| `postProcessManager` | Post-processing. |
| `synthManager` | Synthesizer instruments. |
| `soundManager` | Sound clips. |
| `simulationInput` | Input of the current step. |

Do not keep the contexts after the hook returns; use them inside the hooks.

## Changing scenes

`PushScene` finishes the current scene and `PopScene` destroys the top one. Between scenes the engine cleans
up the screens, the cameras (except the default one), the actors and their colliders, the added
post-processes, the synthesizer instruments and the playing sounds. The scene below a popped scene runs
`OnSetup` again.
