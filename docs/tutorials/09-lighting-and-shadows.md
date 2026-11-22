# Lighting and shadows

The `GlobalLight` is a directional light with an ambient term and a shadow map. It is off by default; when it
is off, models are drawn with their plain colors.

## Turning it on

```cpp
SpinoCore::LaunchOptions options{};
options.enableGlobalLight = true;
options.globalLight.position = {4.0f, 8.0f, 4.0f};
options.globalLight.ambientColor = {0.2f, 0.2f, 0.25f};
```

## Adjusting it in a scene

```cpp
#include "SpinoCore/Core/Lighting/GlobalLight.h"

void OnSetup(const ControlContext& ctx) override {
    ctx.globalLight.SetPosition({1.5f, 8.0f, 6.0f});
    ctx.globalLight.SetTarget({0.0f, 0.0f, 0.0f});
    ctx.globalLight.SetColor({1.0f, 0.95f, 0.85f});
    ctx.globalLight.SetIntensity(1.2f);
    ctx.globalLight.SetShadowExtent(6.0f);
    ctx.globalLight.SetShadowRange(0.1f, 30.0f);
}
```

The light shines from `position` towards `target`. `SetHeight` changes only the height.

## Shadows

The shadow map is a 2048x2048 depth texture rendered from the light with an orthographic projection:

- `shadowExtent` is half the width of the square area that receives shadows. Smaller areas give sharper
  shadows, so fit it to the playing area.
- `shadowNear` and `shadowFar` limit the depth range that casts shadows.

Every model drawn by a `ModelComponent`, animated or not, casts and receives shadows.

## Reading the light

`GetSettings()` returns the current values, `GetDirection()` the unit direction of the light, and
`GetViewProjection()` the matrix used for the shadow map.
