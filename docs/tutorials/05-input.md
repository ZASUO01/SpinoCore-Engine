# Input

`InputState` holds the keyboard and the mouse. The engine keeps two of them:

- the **frame input**, sent to the UI once per rendered frame;
- the **simulation input**, read by scenes (`ctx.simulationInput`) and actors (`OnProcessInput`) once per
  simulation step.

The simulation input accumulates every frame since the last step, so a quick tap is never lost. Input that
arrives while the simulation is paused or loading is discarded.

## Keys

Keys use SDL scancodes.

```cpp
#include <SDL3/SDL_scancode.h>
#include "SpinoCore/Core/Input/InputState.h"

void OnUpdate(const ControlContext& ctx) override {
    const auto& input = ctx.simulationInput;
    if (input.WasKeyPressed(SDL_SCANCODE_ESCAPE)) PauseGame();
    if (input.IsKeyDown(SDL_SCANCODE_LSHIFT)) Run();
}
```

| Call | True when |
|---|---|
| `IsKeyDown` | The key is held. |
| `WasKeyPressed` | The key went down since the last frame or step. |
| `WasKeyReleased` | The key went up since the last frame or step. |
| `WasKeyTyped` | The key was pressed or repeated by the system (text editing). |
| `GetTypedText` | UTF-8 text typed since the last frame or step. |

## Mouse

```cpp
using SpinoCore::Core::Input::MouseButton;

if (input.WasMouseButtonPressed(MouseButton::Left)) Shoot(input.GetMousePosition());
const auto look = input.GetMouseDelta();
const auto zoom = input.GetWheelDelta().y;
```

`IsMouseInWindow` tells if the pointer is over the window. When the window loses focus, every key and button
is released.

## Actors

```cpp
class Player final : public SpinoCore::Core::Actors::Actor {
protected:
    void OnProcessInput(const SpinoCore::Core::Input::InputState& input) override {
        mDirection = {0.0f, 0.0f, 0.0f};
        if (input.IsKeyDown(SDL_SCANCODE_W)) mDirection.z -= 1.0f;
        if (input.IsKeyDown(SDL_SCANCODE_S)) mDirection.z += 1.0f;
    }

    void OnUpdate(const float deltaTime) override {
        SetPosition(GetPosition() + mDirection * (4.0f * deltaTime));
    }

private:
    SpinoCore::Math::Vector3f mDirection{};
};
```

Only actors in the `Active` state receive input.
