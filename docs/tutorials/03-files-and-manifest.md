# Files and manifest

Assets live in an `Assets` folder next to the executable. In CMake, `spinocore_add_assets(<target>)` copies the
`Assets` folder of the project next to the target on every build.

```cmake
target_link_libraries(MyGame PRIVATE SpinoCore::Runtime)
spinocore_add_assets(MyGame)
```

Every path given to the `FileManager` is relative to that folder. Absolute paths are rejected.

## The manifest

```json
{
  "version": "1.0",
  "assets": [
    { "name": "hero", "type": "Texture", "path": "Textures/hero.png" },
    { "name": "title", "type": "Font", "path": "Fonts/JetBrainsMono.ttf" },
    { "name": "character", "type": "Model", "path": "character.glb" },
    { "name": "click", "type": "Sound", "path": "Sounds/click.wav" }
  ]
}
```

- `name`: the key of the asset in its cache.
- `type`: `Texture`, `Font`, `Model` or `Sound`.
- `path`: relative path of the file.

Models also accept `texture` (an image used by every material) and `material` (a JSON file):

```json
{ "version": "1.0", "baseColor": [1.0, 0.8, 0.6, 1.0], "texture": "Textures/skin.png" }
```

An invalid manifest throws `std::runtime_error` from `LoadFromManifest` and nothing is queued.

## Loading

```cpp
void OnStart(const StartContext& ctx) override {
    ctx.fileManager.LoadFromManifest("manifest.json");
    ctx.fileManager.LoadFromSinglePath(SpinoCore::Files::AssetType::Texture, "logo", "Textures/logo.png");
}
```

Requests are processed a few per frame. `OnSetup` runs when `IsReady()` is true. An asset that fails is logged,
listed by `GetFailedLoads()`, and replaced by the default asset of its cache (a white texture, the embedded
font, a cube, a silent clip).

```cpp
void OnSetup(const ControlContext& ctx) override {
    for (const std::string& name : ctx.fileManager.GetFailedLoads()) {
        SpinoCore::Logs::Logger::Warn("Missing asset {}", name);
    }
}
```

## Writing files

```cpp
fileManager.WriteToSinglePath("Saves/slot1.bin", std::vector<std::uint8_t>{1, 2, 3});
```

Writes run on the next processing and replace the file atomically. `FileExists` checks a relative path.
