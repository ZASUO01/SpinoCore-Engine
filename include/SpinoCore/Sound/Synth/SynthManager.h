// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>

namespace SpinoCore::Sound::Synth {
    class Timbre;
    enum class SynthMode : std::uint8_t;

    /// @brief Plays registered instruments on the audio device.
    ///
    /// Calls are queued and applied on the audio thread.
    class SynthManager {
    public:
        virtual ~SynthManager() = default;

        /// @brief Adds an instrument.
        /// @param name Unique name.
        /// @param timbre Sound of the instrument.
        /// @param mode Monophonic or polyphonic.
        /// @return false when the name is empty or used or the timbre is null.
        virtual bool Register(std::string name, std::unique_ptr<Timbre> timbre, SynthMode mode) = 0;
        /// @brief Removes an instrument.
        /// @param name Name of the instrument.
        /// @return Whether it existed.
        virtual bool Unregister(std::string_view name) = 0;

        /// @brief Starts a note on an instrument.
        /// @param name Name of the instrument.
        /// @param midiNote MIDI note from 0 to 127.
        /// @param velocity Loudness from 0 to 1.
        /// @param duration Seconds before the note is released; 0 holds it until NoteOff.
        /// @return Whether the instrument exists and the note is valid.
        virtual bool NoteOn(std::string_view name, int midiNote, float velocity = 1.0f, float duration = 0.0f) = 0;
        /// @brief Releases a note.
        /// @param name Name of the instrument.
        /// @param midiNote MIDI note.
        /// @return Whether the instrument exists.
        virtual bool NoteOff(std::string_view name, int midiNote) = 0;
        /// @brief Starts several notes on an instrument.
        /// @param name Name of the instrument.
        /// @param midiNotes MIDI notes.
        /// @param velocity Loudness from 0 to 1.
        /// @param duration Seconds before the notes are released; 0 holds them.
        /// @return false when the instrument is unknown, the list is empty, has more notes than voices or has a note out of range.
        virtual bool PlayChord(std::string_view name, std::span<const int> midiNotes, float velocity = 1.0f, float duration = 0.0f) = 0;
        /// @brief Releases every note of an instrument.
        /// @param name Name of the instrument.
        /// @return Whether the instrument exists.
        virtual bool Release(std::string_view name) = 0;
        /// @brief Silences every instrument at once.
        virtual void StopAll() = 0;

        /// @brief Changes the volume of an instrument.
        /// @param name Name of the instrument.
        /// @param volume New volume.
        /// @return Whether the instrument exists.
        virtual bool SetVolume(std::string_view name, float volume) = 0;
        /// @brief Changes the volume of every instrument.
        /// @param volume Value clamped to 0..1.
        virtual void SetMasterVolume(float volume) = 0;
        /// @return The master volume.
        [[nodiscard]] virtual float GetMasterVolume() const = 0;

        /// @param name Name of an instrument.
        /// @return Whether it is registered.
        [[nodiscard]] virtual bool Contains(std::string_view name) const = 0;
        /// @brief Removes every instrument; the engine calls it between scenes.
        virtual void CleanUp() = 0;
    };
}
