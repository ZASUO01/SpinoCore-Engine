# Post-processing

A post-process is a fragment shader applied to the whole rendered scene before the UI is drawn. One
post-process can be active at a time.

## Built-in effects

| Name | Uniforms |
|---|---|
| `DefaultPostProcess::Grayscale` | 0: strength (0..1). |
| `DefaultPostProcess::Vignette` | 0: radius, 1: softness. |
| `DefaultPostProcess::Exposure` | 0: exposure, 1: gamma. |

```cpp
#include "SpinoCore/Graphics/PostProcess.h"
#include "SpinoCore/Graphics/PostProcessManager.h"

void OnSetup(const ControlContext& ctx) override {
    ctx.postProcessManager.SetCurrent(SpinoCore::Graphics::DefaultPostProcess::Vignette);
    ctx.postProcessManager.ToggleActive();
}

void OnUpdate(const ControlContext& ctx) override {
    if (ctx.simulationInput.WasKeyPressed(SDL_SCANCODE_P)) ctx.postProcessManager.ToggleActive();
}
```

## Writing an effect

Write only `main`. The engine adds the GLSL version, the input `fragTextureCoord`, the output `outColor`, and
the uniforms `uScene` (the scene texture), `uResolution` (pixels) and `uTime` (seconds). User uniforms use
locations 0, 1, 2 and so on, in the order of the list.

```cpp
using namespace SpinoCore::Graphics;

PostProcess tint{
    .fragmentShader = R"(
        layout (location = 0) uniform vec4 uTint;
        void main() {
            vec4 color = texture(uScene, fragTextureCoord);
            outColor = vec4(color.rgb * uTint.rgb, color.a);
        }
    )",
    .uniforms = {PostProcessUniform{.type = UniformType::Vec4, .value = SpinoCore::Math::Vector4f{1.0f, 0.6f, 0.6f, 1.0f}}},
};

ctx.postProcessManager.AddPostProcess("Tint", std::move(tint));
ctx.postProcessManager.SetCurrent("Tint");
ctx.postProcessManager.ToggleActive();
```

`AddPostProcess` returns false when the name is empty or in use, when there are more than 100 uniforms, or when
a value does not match its declared type. The shader is compiled when it is first used; a compile error is
logged.

## Changing uniforms

```cpp
ctx.postProcessManager.SetUniform("Tint", 0, SpinoCore::Math::Vector4f{0.6f, 0.6f, 1.0f, 1.0f});
```

The value must have the declared type. Post-processes added by a scene are removed when the scene changes;
the built-in ones stay.
