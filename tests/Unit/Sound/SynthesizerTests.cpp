// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <memory>
#include <vector>
#include <gtest/gtest.h>
#include "SpinoCore/Sound/Synth/Synthesizer.h"
#include "SpinoCore/Sound/Synth/SynthUtils.h"
#include "SpinoCore/Sound/Synth/Timbre.h"

namespace {
    using SpinoCore::Sound::Synth::MaxVoices;
    using SpinoCore::Sound::Synth::MidiToFrequency;
    using SpinoCore::Sound::Synth::Note;
    using SpinoCore::Sound::Synth::SynthMode;
    using SpinoCore::Sound::Synth::Synthesizer;
    using SpinoCore::Sound::Synth::Timbre;
    using SpinoCore::Sound::Synth::ToMidi;

    constexpr int SampleRate = 48000;
    constexpr int A4 = 69;

    class PlainTimbre final : public Timbre {
    public:
        explicit PlainTimbre(const float attack = 0.0f, const float release = 0.01f) : mAttack(attack), mRelease(release) {}

        void SetupInstrument() override {
            mConfig.attack = mAttack;
            mConfig.release = mRelease;
            mConfig.sustain = 1.0f;
            mConfig.volume = 1.0f;
        }

    private:
        float mAttack;
        float mRelease;
    };

    [[nodiscard]] std::unique_ptr<Timbre> Plain(const float attack = 0.0f, const float release = 0.01f) {
        auto timbre = std::make_unique<PlainTimbre>(attack, release);
        timbre->SetupInstrument();
        return timbre;
    }

    [[nodiscard]] std::vector<float> Render(Synthesizer& synth, const float seconds) {
        std::vector<float> samples(static_cast<std::size_t>(seconds * SampleRate) * 2, 0.0f);
        synth.Render(samples);
        return samples;
    }

    [[nodiscard]] int ZeroCrossingsOnTheLeft(const std::vector<float>& samples) {
        int crossings = 0;
        for (std::size_t index = 2; index < samples.size(); index += 2) {
            if ((samples[index - 2] < 0.0f) != (samples[index] < 0.0f)) ++crossings;
        }
        return crossings;
    }

    [[nodiscard]] float Peak(const std::vector<float>& samples) {
        float peak = 0.0f;
        for (const float sample : samples) peak = std::max(peak, std::abs(sample));
        return peak;
    }

    [[nodiscard]] float LargestStep(const std::vector<float>& before, const std::vector<float>& after) {
        std::vector<float> joined = before;
        joined.insert(joined.end(), after.begin(), after.end());
        float largest = 0.0f;
        for (std::size_t index = 2; index < joined.size(); index += 2) largest = std::max(largest, std::abs(joined[index] - joined[index - 2]));
        return largest;
    }

    TEST(SynthUtilsTests, A4IsTheMidiNote69) {
        EXPECT_EQ(ToMidi(Note::A, 4), 69);
    }

    TEST(SynthUtilsTests, A4SoundsAt440Hertz) {
        EXPECT_FLOAT_EQ(MidiToFrequency(69), 440.0f);
    }

    TEST(SynthUtilsTests, OneOctaveUpDoublesTheFrequency) {
        EXPECT_NEAR(MidiToFrequency(ToMidi(Note::C, 5)), 2.0f * MidiToFrequency(ToMidi(Note::C, 4)), 1e-3f);
    }

    TEST(SynthesizerTests, ANoteSoundsAtItsFrequency) {
        Synthesizer synth{Plain(), SynthMode::Polyphonic, SampleRate};
        synth.NoteOn(A4);

        const auto samples = Render(synth, 1.0f);

        EXPECT_NEAR(ZeroCrossingsOnTheLeft(samples), 880, 4);
    }

    TEST(SynthesizerTests, ReleasingANoteFadesToSilence) {
        Synthesizer synth{Plain(0.0f, 0.01f), SynthMode::Polyphonic, SampleRate};
        synth.NoteOn(A4);
        static_cast<void>(Render(synth, 0.1f));
        synth.NoteOff(A4);
        static_cast<void>(Render(synth, 0.02f));

        const auto afterRelease = Render(synth, 0.05f);

        EXPECT_EQ(Peak(afterRelease), 0.0f);
    }

    TEST(SynthesizerTests, ANoteWithADurationReleasesItself) {
        Synthesizer synth{Plain(0.0f, 0.01f), SynthMode::Polyphonic, SampleRate};
        synth.NoteOn(A4, 1.0f, 0.05f);
        static_cast<void>(Render(synth, 0.1f));

        EXPECT_EQ(synth.GetActiveVoiceCount(), 0u);
    }

    TEST(SynthesizerTests, AChordOfThreeNotesUsesThreeVoices) {
        Synthesizer synth{Plain(), SynthMode::Polyphonic, SampleRate};
        const std::array chord{60, 64, 67};

        synth.PlayChord(chord);

        EXPECT_EQ(synth.GetActiveVoiceCount(), 3u);
    }

    TEST(SynthesizerTests, AChordLargerThanTheVoiceLimitIsRejected) {
        Synthesizer synth{Plain(), SynthMode::Polyphonic, SampleRate};
        const std::array chord{60, 62, 64, 65, 67, 69};

        const bool played = synth.PlayChord(chord);

        EXPECT_FALSE(played);
        EXPECT_EQ(synth.GetActiveVoiceCount(), 0u);
    }

    TEST(SynthesizerTests, ASixthNoteTakesTheVoiceOfTheOldest) {
        Synthesizer synth{Plain(0.0f, 0.01f), SynthMode::Polyphonic, SampleRate};
        for (int note = 60; note < 65; ++note) synth.NoteOn(note);
        synth.NoteOn(65);

        synth.NoteOff(60);
        static_cast<void>(Render(synth, 0.05f));

        EXPECT_EQ(synth.GetActiveVoiceCount(), MaxVoices);
    }

    TEST(SynthesizerTests, PlayingTheSameNoteAgainReusesItsVoice) {
        Synthesizer synth{Plain(), SynthMode::Polyphonic, SampleRate};
        synth.NoteOn(60);

        synth.NoteOn(60);

        EXPECT_EQ(synth.GetActiveVoiceCount(), 1u);
    }

    TEST(SynthesizerTests, AMonophonicSynthKeepsOnlyTheLatestNote) {
        Synthesizer synth{Plain(), SynthMode::Monophonic, SampleRate};
        synth.NoteOn(60);

        synth.NoteOn(A4);

        EXPECT_EQ(synth.GetActiveVoiceCount(), 1u);
        EXPECT_NEAR(ZeroCrossingsOnTheLeft(Render(synth, 1.0f)), 880, 4);
    }

    TEST(SynthesizerTests, AMonophonicSynthIgnoresTheReleaseOfAReplacedNote) {
        Synthesizer synth{Plain(0.0f, 0.01f), SynthMode::Monophonic, SampleRate};
        synth.NoteOn(60);
        synth.NoteOn(A4);

        synth.NoteOff(60);
        static_cast<void>(Render(synth, 0.05f));

        EXPECT_EQ(synth.GetActiveVoiceCount(), 1u);
    }

    TEST(SynthesizerTests, AMonophonicSynthRejectsChords) {
        Synthesizer synth{Plain(), SynthMode::Monophonic, SampleRate};
        const std::array chord{60, 64};

        EXPECT_FALSE(synth.PlayChord(chord));
    }

    TEST(SynthesizerTests, RetriggeringANoteDoesNotClick) {
        Synthesizer synth{Plain(), SynthMode::Polyphonic, SampleRate};
        synth.NoteOn(A4);
        const auto before = Render(synth, 0.013f);

        synth.NoteOn(A4);
        const auto after = Render(synth, 0.01f);

        EXPECT_LT(LargestStep(before, after), 0.1f);
    }

    TEST(SynthesizerTests, ReleasingDuringTheAttackDoesNotJump) {
        Synthesizer synth{Plain(0.5f, 0.2f), SynthMode::Monophonic, SampleRate};
        synth.NoteOn(A4);
        const auto before = Render(synth, 0.1f);

        synth.NoteOff(A4);
        const auto after = Render(synth, 0.01f);

        EXPECT_LT(LargestStep(before, after), 0.1f);
    }

    TEST(SynthesizerTests, StopAllSilencesImmediately) {
        Synthesizer synth{Plain(0.0f, 1.0f), SynthMode::Polyphonic, SampleRate};
        synth.NoteOn(A4);

        synth.StopAll();

        EXPECT_EQ(Peak(Render(synth, 0.01f)), 0.0f);
    }

    TEST(SynthesizerTests, AnOutOfRangeNoteIsRejected) {
        Synthesizer synth{Plain(), SynthMode::Polyphonic, SampleRate};

        EXPECT_FALSE(synth.NoteOn(128));
    }
}
