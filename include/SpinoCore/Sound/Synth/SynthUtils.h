// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cmath>

namespace SpinoCore::Sound::Synth {
    enum class WaveformType {
        Sine,
        Square,
        Sawtooth,
        Triangle,
    };

    enum class Note {
        C, CS, D, DS, E, F, FS, G, GS, A, AS, B, Count, None
    };

    inline constexpr int MinMidiNote = 0;
    inline constexpr int MaxMidiNote = 127;
    inline constexpr int InvalidMidiNote = -1;

    [[nodiscard]] constexpr int ToMidi(const Note note, const int octave) noexcept {
        if (note == Note::None || note == Note::Count) return InvalidMidiNote;
        const int midi = (octave + 1) * 12 + static_cast<int>(note);
        return midi >= MinMidiNote && midi <= MaxMidiNote ? midi : InvalidMidiNote;
    }

    [[nodiscard]] inline float MidiToFrequency(const int midiNote) noexcept {
        return 440.0f * std::pow(2.0f, static_cast<float>(midiNote - 69) / 12.0f);
    }
}
