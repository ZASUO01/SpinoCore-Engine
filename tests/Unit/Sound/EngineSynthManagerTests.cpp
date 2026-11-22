// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <memory>
#include <vector>
#include <gtest/gtest.h>
#include "Sound/Synth/EngineSynthManager.h"
#include "SpinoCore/Sound/Synth/SynthManager.h"
#include "SpinoCore/Sound/Synth/Synthesizer.h"
#include "SpinoCore/Sound/Synth/SynthUtils.h"
#include "SpinoCore/Sound/Synth/Timbre.h"

namespace {
    using SpinoCore::Sound::Synth::AudioOutput;
    using SpinoCore::Sound::Synth::EngineSynthManager;
    using SpinoCore::Sound::Synth::SynthManager;
    using SpinoCore::Sound::Synth::SynthMode;
    using SpinoCore::Sound::Synth::Timbre;
    using SpinoCore::Sound::Synth::WaveformType;

    class LoudSaw final : public Timbre {
    public:
        void SetupInstrument() override {
            mConfig.osc1Type = WaveformType::Sawtooth;
            mConfig.release = 0.01f;
            mConfig.volume = 1.0f;
        }
    };

    [[nodiscard]] std::unique_ptr<Timbre> Saw() {
        return Timbre::CreateUnique<LoudSaw>();
    }

    [[nodiscard]] std::vector<float> Mix(EngineSynthManager& manager, const std::size_t frames = 1024) {
        std::vector<float> samples(frames * 2, 0.0f);
        manager.RenderMix(samples);
        return samples;
    }

    [[nodiscard]] float Peak(const std::vector<float>& samples) {
        float peak = 0.0f;
        for (const float sample : samples) peak = std::max(peak, std::abs(sample));
        return peak;
    }

    class EngineSynthManagerTests : public testing::Test {
    protected:
        EngineSynthManager manager{AudioOutput::Offline};
        SynthManager& synths{manager};
    };

    TEST_F(EngineSynthManagerTests, ARegisteredInstrumentIsAvailable) {
        manager.Register("pad", Saw(), SynthMode::Polyphonic);

        EXPECT_TRUE(manager.Contains("pad"));
    }

    TEST_F(EngineSynthManagerTests, ANameCanOnlyBeRegisteredOnce) {
        manager.Register("pad", Saw(), SynthMode::Polyphonic);

        EXPECT_FALSE(manager.Register("pad", Saw(), SynthMode::Monophonic));
    }

    TEST_F(EngineSynthManagerTests, PlayingAnUnknownInstrumentFails) {
        EXPECT_FALSE(synths.NoteOn("ghost", 60));
    }

    TEST_F(EngineSynthManagerTests, ANoteWaitsForTheNextAudioBlock) {
        manager.Register("lead", Saw(), SynthMode::Monophonic);

        synths.NoteOn("lead", 60);

        EXPECT_EQ(manager.Find("lead")->GetActiveVoiceCount(), 0u);
    }

    TEST_F(EngineSynthManagerTests, ANoteStartsInTheNextAudioBlock) {
        manager.Register("lead", Saw(), SynthMode::Monophonic);
        synths.NoteOn("lead", 60);

        static_cast<void>(Mix(manager));

        EXPECT_EQ(manager.Find("lead")->GetActiveVoiceCount(), 1u);
    }

    TEST_F(EngineSynthManagerTests, APlayingNoteProducesSound) {
        manager.Register("lead", Saw(), SynthMode::Monophonic);
        synths.NoteOn("lead", 60);

        EXPECT_GT(Peak(Mix(manager)), 0.0f);
    }

    TEST_F(EngineSynthManagerTests, InstrumentsPlayIndependently) {
        manager.Register("pad", Saw(), SynthMode::Polyphonic);
        manager.Register("lead", Saw(), SynthMode::Monophonic);
        const std::array chord{60, 64, 67};
        synths.PlayChord("pad", chord);

        static_cast<void>(Mix(manager));

        EXPECT_EQ(manager.Find("pad")->GetActiveVoiceCount(), 3u);
        EXPECT_EQ(manager.Find("lead")->GetActiveVoiceCount(), 0u);
    }

    TEST_F(EngineSynthManagerTests, TheMixNeverLeavesTheValidRange) {
        manager.Register("pad", Saw(), SynthMode::Polyphonic);
        manager.Register("lead", Saw(), SynthMode::Monophonic);
        const std::array chord{48, 52, 55, 60, 64};
        synths.PlayChord("pad", chord);
        synths.NoteOn("lead", 72);

        const auto samples = Mix(manager, 4800);

        EXPECT_LE(Peak(samples), 1.0f);
    }

    TEST_F(EngineSynthManagerTests, AChordLargerThanTheVoiceLimitIsRejected) {
        manager.Register("pad", Saw(), SynthMode::Polyphonic);
        const std::array chord{60, 62, 64, 65, 67, 69};

        EXPECT_FALSE(synths.PlayChord("pad", chord));
    }

    TEST_F(EngineSynthManagerTests, AMasterVolumeOfZeroSilencesTheMix) {
        manager.Register("lead", Saw(), SynthMode::Monophonic);
        synths.NoteOn("lead", 60);

        manager.SetMasterVolume(0.0f);

        EXPECT_EQ(Peak(Mix(manager)), 0.0f);
    }

    TEST_F(EngineSynthManagerTests, AnInstrumentVolumeOfZeroSilencesIt) {
        manager.Register("lead", Saw(), SynthMode::Monophonic);
        synths.NoteOn("lead", 60);

        manager.SetVolume("lead", 0.0f);

        EXPECT_EQ(Peak(Mix(manager)), 0.0f);
    }

    TEST_F(EngineSynthManagerTests, StopAllSilencesEveryInstrument) {
        manager.Register("lead", Saw(), SynthMode::Monophonic);
        synths.NoteOn("lead", 60);
        static_cast<void>(Mix(manager));

        manager.StopAll();

        EXPECT_EQ(Peak(Mix(manager)), 0.0f);
    }

    TEST_F(EngineSynthManagerTests, CleanUpRemovesEveryInstrument) {
        manager.Register("lead", Saw(), SynthMode::Monophonic);

        manager.CleanUp();

        EXPECT_FALSE(manager.Contains("lead"));
    }

    TEST_F(EngineSynthManagerTests, CleanUpSilencesTheMix) {
        manager.Register("lead", Saw(), SynthMode::Monophonic);
        synths.NoteOn("lead", 60);
        static_cast<void>(Mix(manager));

        manager.CleanUp();

        EXPECT_EQ(Peak(Mix(manager)), 0.0f);
    }

    TEST_F(EngineSynthManagerTests, UnregisteringWithPendingNotesIsSafe) {
        manager.Register("lead", Saw(), SynthMode::Monophonic);
        synths.NoteOn("lead", 60);

        manager.Unregister("lead");

        EXPECT_EQ(Peak(Mix(manager)), 0.0f);
    }

    TEST(EngineSynthManagerWithoutDeviceTests, WithoutAnAudioDeviceItRunsSilently) {
        EngineSynthManager manager{AudioOutput::Device};

        EXPECT_FALSE(manager.HasDevice());
    }

    TEST(EngineSynthManagerWithoutDeviceTests, WithoutAnAudioDeviceNotesStillReachTheInstrument) {
        EngineSynthManager manager{AudioOutput::Device};
        SynthManager& synths = manager;
        manager.Register("lead", Saw(), SynthMode::Monophonic);

        synths.NoteOn("lead", 60);

        EXPECT_EQ(manager.Find("lead")->GetActiveVoiceCount(), 1u);
    }
}
