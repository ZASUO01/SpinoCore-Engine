# Synth

The synthesizer plays notes in real time with no audio files. Each instrument is a `Timbre` (the sound) played
by a monophonic or polyphonic synthesizer. Calls are queued and applied on the audio thread.

## A timbre

```cpp
#include "SpinoCore/Sound/Synth/Timbre.h"

class PadTimbre final : public SpinoCore::Sound::Synth::Timbre {
public:
    void SetupInstrument() override {
        mConfig.osc1Type = SpinoCore::Sound::Synth::WaveformType::Sawtooth;
        mConfig.osc2Type = SpinoCore::Sound::Synth::WaveformType::Triangle;
        mConfig.oscMix = 0.5f;
        mConfig.attack = 0.25f;
        mConfig.decay = 0.4f;
        mConfig.sustain = 0.7f;
        mConfig.release = 0.8f;
        mConfig.filterCutoff = 1600.0f;
        mConfig.reverbSize = 0.6f;
        mConfig.reverbMix = 0.3f;
    }
};
```

| Group | Fields |
|---|---|
| Oscillators | `osc1Type`, `osc2Type` (sine, square, sawtooth, triangle), `oscMix`, `detune` |
| Vibrato | `lfoRate`, `lfoAmount` |
| Filter | `filterCutoff`, `filterResonance`, `filterEnvAmount` |
| Envelope | `attack`, `decay`, `sustain`, `release` |
| Effects | `delayTime`, `delayFeedback`, `delayMix`, `reverbSize`, `reverbMix` |
| Level | `volume` |

## Registering and playing

```cpp
#include "SpinoCore/Sound/Synth/SynthManager.h"
#include "SpinoCore/Sound/Synth/Synthesizer.h"
#include "SpinoCore/Sound/Synth/SynthUtils.h"

using namespace SpinoCore::Sound::Synth;

void OnSetup(const ControlContext& ctx) override {
    ctx.synthManager.Register("pad", Timbre::CreateUnique<PadTimbre>(), SynthMode::Polyphonic);
    ctx.synthManager.SetVolume("pad", 0.5f);
    ctx.synthManager.SetMasterVolume(0.6f);
}

void OnUpdate(const ControlContext& ctx) override {
    if (ctx.simulationInput.WasKeyPressed(SDL_SCANCODE_SPACE)) {
        constexpr std::array chord{48, 52, 55};
        ctx.synthManager.PlayChord("pad", chord, 0.8f, 1.5f);
        ctx.synthManager.NoteOn("pad", ToMidi(Note::G, 4), 1.0f, 0.5f);
    }
}
```

Notes are MIDI numbers (C4 is 60); `ToMidi(Note, octave)` converts names. The last argument is the duration in
seconds; 0 holds the note until `NoteOff` or `Release`.

- Monophonic instruments play one note at a time; polyphonic ones play up to 5.
- `StopAll` silences everything at once.
- While sound clips play, the synthesizer volume is lowered so the clips stand out.
- Instruments registered by a scene are removed when the scene changes.
