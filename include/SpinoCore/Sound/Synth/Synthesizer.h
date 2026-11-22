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

    enum class SynthMode : std::uint8_t {
        Monophonic,
        Polyphonic
    };

    inline constexpr std::size_t MaxVoices = 5;

    class Synthesizer {
    public:
        explicit Synthesizer(std::unique_ptr<Timbre> timbre, SynthMode mode = SynthMode::Polyphonic, int sampleRate = 48000);
        ~Synthesizer();

        Synthesizer(const Synthesizer&) = delete;
        Synthesizer& operator=(const Synthesizer&) = delete;
        Synthesizer(Synthesizer&&) = delete;
        Synthesizer& operator=(Synthesizer&&) = delete;

        void SetSampleRate(int sampleRate);
        void SetTimbre(std::unique_ptr<Timbre> timbre);

        bool NoteOn(int midiNote, float velocity = 1.0f, float duration = 0.0f);
        bool PlayNote(Note note, int octave, float duration);
        void NoteOff(int midiNote);
        bool PlayChord(std::span<const int> midiNotes, float velocity = 1.0f, float duration = 0.0f);
        void ReleaseAll();
        void StopAll();

        void Render(std::span<float> interleavedStereo);

        void SetVolume(const float volume) { mVolume = volume < 0.0f ? 0.0f : volume; }
        [[nodiscard]] float GetVolume() const { return mVolume; }
        [[nodiscard]] SynthMode GetMode() const { return mMode; }
        [[nodiscard]] std::size_t GetVoiceCapacity() const { return mVoiceCapacity; }
        [[nodiscard]] std::size_t GetActiveVoiceCount() const;
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
