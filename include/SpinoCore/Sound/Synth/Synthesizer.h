// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include "SynthUtils.h"

namespace SpinoCore::Sound::Synth {
    class Timbre;

    /// @brief How many notes an instrument plays at once.
    enum class SynthMode : std::uint8_t {
        /// One note; a new note replaces the current one.
        Monophonic,
        /// Up to MaxVoices notes.
        Polyphonic
    };

    /// Number of voices of a polyphonic synthesizer.
    inline constexpr std::size_t MaxVoices = 5;

    /// @brief Renders the notes of one instrument into a stereo buffer.
    ///
    /// Games use it through the SynthManager, which runs it on the audio thread.
    class Synthesizer {
    public:
        /// @brief Creates a synthesizer.
        /// @param timbre Sound of the instrument.
        /// @param mode Monophonic or polyphonic.
        /// @param sampleRate Output sample rate in hertz.
        explicit Synthesizer(std::unique_ptr<Timbre> timbre, SynthMode mode = SynthMode::Polyphonic, int sampleRate = 48000);
        /// @brief Destroys the synthesizer and its timbre.
        ~Synthesizer();

        Synthesizer(const Synthesizer&) = delete;
        Synthesizer& operator=(const Synthesizer&) = delete;
        Synthesizer(Synthesizer&&) = delete;
        Synthesizer& operator=(Synthesizer&&) = delete;

        /// @brief Changes the output sample rate.
        /// @param sampleRate Sample rate in hertz.
        void SetSampleRate(int sampleRate);
        /// @brief Replaces the instrument sound and silences every voice.
        /// @param timbre New timbre.
        void SetTimbre(std::unique_ptr<Timbre> timbre);

        /// @brief Starts a note.
        /// @param midiNote MIDI note from 0 to 127.
        /// @param velocity Loudness from 0 to 1.
        /// @param duration Seconds before the note is released; 0 holds it until NoteOff.
        /// @return false when the note is out of range or there is no timbre.
        bool NoteOn(int midiNote, float velocity = 1.0f, float duration = 0.0f);
        /// @brief Starts a note by name at full velocity.
        /// @param note Note name.
        /// @param octave Octave number.
        /// @param duration Seconds before the note is released; 0 holds it.
        /// @return Whether the note started.
        bool PlayNote(Note note, int octave, float duration);
        /// @brief Releases a note; it fades out with the release time.
        /// @param midiNote MIDI note.
        void NoteOff(int midiNote);
        /// @brief Starts several notes at once.
        /// @param midiNotes MIDI notes.
        /// @param velocity Loudness from 0 to 1.
        /// @param duration Seconds before the notes are released; 0 holds them.
        /// @return false when the list is empty, has more notes than voices or has a note out of range.
        bool PlayChord(std::span<const int> midiNotes, float velocity = 1.0f, float duration = 0.0f);
        /// @brief Releases every note.
        void ReleaseAll();
        /// @brief Silences every voice at once.
        void StopAll();

        /// @brief Adds the next samples of every voice to a buffer.
        /// @param interleavedStereo Buffer of left and right samples.
        void Render(std::span<float> interleavedStereo);

        /// @brief Changes the instrument volume.
        /// @param volume New volume; negative values become 0.
        void SetVolume(const float volume) { mVolume = volume < 0.0f ? 0.0f : volume; }
        /// @return The instrument volume.
        [[nodiscard]] float GetVolume() const { return mVolume; }
        /// @return Monophonic or polyphonic.
        [[nodiscard]] SynthMode GetMode() const { return mMode; }
        /// @return The number of notes that can sound at once.
        [[nodiscard]] std::size_t GetVoiceCapacity() const { return mVoiceCapacity; }
        /// @return The number of voices that are sounding.
        [[nodiscard]] std::size_t GetActiveVoiceCount() const;
        /// @return The output sample rate in hertz.
        [[nodiscard]] int GetSampleRate() const { return mSampleRate; }

    private:
        enum class EnvelopeStage : std::uint8_t { Idle, Attack, Decay, Sustain, Release };

        struct Voice {
            int note{InvalidMidiNote};
            float frequency{0.0f};
            float velocity{0.0f};
            double phaseL{0.0};
            double phaseR{0.0};
            EnvelopeStage stage{EnvelopeStage::Idle};
            float level{0.0f};
            float releaseRate{0.0f};
            float lfoPhase{0.0f};
            float filterL{0.0f};
            float filterR{0.0f};
            float remaining{0.0f};
            std::uint64_t startedAt{0};
        };

        [[nodiscard]] Voice& VoiceFor(int midiNote);
        void Start(Voice& voice, int midiNote, float velocity, float duration);
        void Release(Voice& voice) const;
        void StepEnvelope(Voice& voice) const;
        void RenderVoice(Voice& voice, float& sampleL, float& sampleR) const;

        std::unique_ptr<Timbre> mTimbre;
        SynthMode mMode;
        std::size_t mVoiceCapacity;
        std::array<Voice, MaxVoices> mVoices{};
        int mSampleRate;
        float mSampleTimeStep;
        float mVolume{1.0f};
        std::uint64_t mClock{0};
    };
}
