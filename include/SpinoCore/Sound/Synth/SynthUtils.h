// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cmath>

/// @brief Subtractive synthesizer with oscillators, filter, envelope and effects.
namespace SpinoCore::Sound::Synth {
    /// @brief Oscillator waveform.
    enum class WaveformType {
        Sine,
        Square,
        Sawtooth,
        Triangle,
    };

    /// @brief Note names; the S suffix means sharp.
    enum class Note {
        C, CS, D, DS, E, F, FS, G, GS, A, AS, B, Count, None
    };

    /// Lowest MIDI note.
    inline constexpr int MinMidiNote = 0;
    /// Highest MIDI note.
    inline constexpr int MaxMidiNote = 127;
    /// Returned by ToMidi for notes outside the MIDI range.
    inline constexpr int InvalidMidiNote = -1;

    /// @brief Converts a note name and octave to a MIDI note; C4 is 60.
    /// @param note Note name.
    /// @param octave Octave number.
    /// @return The MIDI note, or InvalidMidiNote.
    [[nodiscard]] constexpr int ToMidi(const Note note, const int octave) noexcept {
        if (note == Note::None || note == Note::Count) return InvalidMidiNote;
        const int midi = (octave + 1) * 12 + static_cast<int>(note);
        return midi >= MinMidiNote && midi <= MaxMidiNote ? midi : InvalidMidiNote;
    }

    /// @param midiNote MIDI note.
    /// @return The frequency in hertz, with A4 (69) at 440 Hz.
    [[nodiscard]] inline float MidiToFrequency(const int midiNote) noexcept {
        return 440.0f * std::pow(2.0f, static_cast<float>(midiNote - 69) / 12.0f);
    }
}
