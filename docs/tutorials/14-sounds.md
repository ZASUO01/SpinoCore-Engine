# Sounds

The `SoundManager` plays audio clips from the sound cache. Clips are decoded when they load, through the
built-in decoders of SDL3_mixer (such as WAV, OGG, MP3 and FLAC), and converted to 48 kHz stereo.

## Loading

```json
{ "name": "click", "type": "Sound", "path": "Sounds/click.wav" }
```

## Playing

```cpp
#include "SpinoCore/Sound/SoundManager.h"

void OnSetup(const ControlContext& ctx) override {
    ctx.soundManager.SetMasterVolume(0.8f);
    mMusic = ctx.soundManager.Play("theme", true, 0.5f);
}

void OnUpdate(const ControlContext& ctx) override {
    if (ctx.simulationInput.WasKeyPressed(SDL_SCANCODE_SPACE)) ctx.soundManager.Play("click");
    if (ctx.simulationInput.WasKeyPressed(SDL_SCANCODE_M)) ctx.soundManager.Pause(mMusic);
}
```

`Play(clip, loop, volume)` returns a `SoundHandle`. An unknown clip is logged and gives an invalid handle.

| Call | Effect |
|---|---|
| `PlayFor(clip, seconds, loop, volume)` | Plays for a fixed time, looping the clip if `loop` is true. |
| `Stop`, `Pause`, `Resume` | Control one sound. |
| `IsPlaying`, `IsAnyPlaying` | Check playback. |
| `SetVolume(handle, volume)` | Changes one sound. |
| `SetMasterVolume`, `StopAll` | Affect every sound. |

## Handles and slots

Up to 16 sounds play at once. When every slot is busy, a new sound replaces the oldest one, and the handles of
replaced or finished sounds stop working: calls with them return false. Keep handles only for sounds you need
to control.

## From the UI

Screens can receive the sound manager in their constructor:

```cpp
ctx.screensManager.PushScreen(std::make_unique<MenuScreen>(ctx.soundManager));

button->SetOnClick([&sounds] { sounds.Play("click"); });
```

Every sound is stopped when the scene changes.
