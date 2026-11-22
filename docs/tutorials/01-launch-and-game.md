# Launch and game

A SpinoCore game is a `GameApplication` subclass handed to `SpinoCore::Launch`. The engine creates the
window, the renderer and every module, calls `Initialize` once, and runs the loop until the window closes.

## The game class

```cpp
#include "SpinoCore/Core/Scenes/ScenesManager.h"
#include "SpinoCore/Game/GameApplication.h"
#include "SpinoCore/Game/InitializationContext.h"
#include "Scenes/MenuScene.h"

class MyGame final : public SpinoCore::Game::GameApplication {
public:
    void Initialize(const SpinoCore::Game::InitializationContext& ctx) override {
        ctx.scenesManager.PushScene(std::make_unique<MenuScene>());
    }
};
```

`Initialize` receives the window manager and the scenes manager. Push the first scene there.

## main

```cpp
#include "SpinoCore/Launch.h"
#include "SpinoCore/LaunchOptions.h"

int main() {
    SpinoCore::LaunchOptions options{};
    options.gameInfo.name = "My Game";
    options.windowProps.title = "My Game";
    options.windowProps.size = SpinoCore::Core::Window::WINDOW_SIZE_MEDIUM;
    options.enableGlobalLight = true;

    return SpinoCore::Launch(std::make_unique<MyGame>(), options);
}
```

`Launch` returns `EXIT_SUCCESS` or `EXIT_FAILURE`. It never throws: startup errors are logged. Only one engine
can run at a time.

## Launch options

| Field | Meaning |
|---|---|
| `gameInfo` | Name, version, identifier, creator and URL given to SDL. |
| `windowProps` | Title, size, mode and whether the system cursor is hidden. |
| `enableGlobalLight` | Turns on lighting and shadows. |
| `globalLight` | Initial light settings. |
| `debugColliders` | Draws collider outlines. |

A third argument, `Logs::LogOptions`, sets the log level and sink (see [Logs](15-logs.md)).

## The frame

Each frame the engine polls events, sends the frame input to the UI, loads pending files and, once the
current scene is ready, runs the simulation in fixed steps of 1/60 s: actor input, scene update, cameras,
actors and collisions. Then it draws the shadows, the models, the particles, the debug shapes, the UI and the
post-process.
