// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <memory>
#include <span>
#include <vector>
#include <gtest/gtest.h>
#include "Assets/AssetCache.h"
#include "Sound/EngineSoundManager.h"
#include "Sound/Resources/AudioClip.h"
#include "Sound/Synth/EngineSynthManager.h"
#include "SpinoCore/Sound/SoundManager.h"

namespace {
    using SpinoCore::Assets::AssetCache;
    using SpinoCore::Sound::EngineSoundManager;
    using SpinoCore::Sound::SoundHandle;
    using SpinoCore::Sound::SoundManager;
    using SpinoCore::Sound::Resources::AudioClip;
    using SpinoCore::Sound::Resources::ClipSampleRate;
    using SpinoCore::Sound::Synth::AudioOutput;

    [[nodiscard]] std::unique_ptr<const AudioClip> ConstantClip(const float seconds, const float value = 0.5f) {
        const auto frames = static_cast<std::size_t>(seconds * static_cast<float>(ClipSampleRate));
        return std::make_unique<const AudioClip>(std::vector<float>(frames * 2, value));
    }

    [[nodiscard]] std::size_t Frames(const float seconds) {
        return static_cast<std::size_t>(seconds * static_cast<float>(ClipSampleRate));
    }

    [[nodiscard]] float Peak(const std::span<const float> samples) {
        float peak = 0.0f;
        for (const float sample : samples) peak = std::max(peak, std::abs(sample));
        return peak;
    }

    class EngineSoundManagerTests : public testing::Test {
    protected:
        void SetUp() override {
            clips.Add("click", ConstantClip(0.05f));
            clips.Add("music", ConstantClip(1.0f));
        }

        std::vector<float> Mix(const float seconds) {
            std::vector<float> samples(Frames(seconds) * 2, 0.0f);
            manager.RenderMix(samples);
            return samples;
        }

        AssetCache<AudioClip> clips{std::make_unique<const AudioClip>()};
        EngineSoundManager manager{&clips, AudioOutput::Offline};
        SoundManager& sounds{manager};
    };

    TEST_F(EngineSoundManagerTests, PlayingALoadedClipProducesSound) {
        sounds.Play("click");

        EXPECT_GT(Peak(Mix(0.01f)), 0.0f);
    }

    TEST_F(EngineSoundManagerTests, PlayingAnUnknownClipGivesAnInvalidHandle) {
        const SoundHandle handle = sounds.Play("ghost");

        EXPECT_FALSE(handle.IsValid());
    }

    TEST_F(EngineSoundManagerTests, AClipStopsAtItsEnd) {
        const SoundHandle handle = sounds.Play("click");

        static_cast<void>(Mix(0.1f));

        EXPECT_FALSE(sounds.IsPlaying(handle));
    }

    TEST_F(EngineSoundManagerTests, AClipIsSilentAfterItsEnd) {
        sounds.Play("click");

        const auto samples = Mix(0.1f);

        EXPECT_EQ(Peak(std::span(samples).subspan(Frames(0.06f) * 2)), 0.0f);
    }

    TEST_F(EngineSoundManagerTests, ALoopingClipStartsAgain) {
        const SoundHandle handle = sounds.Play("click", true);

        const auto samples = Mix(0.2f);

        EXPECT_TRUE(sounds.IsPlaying(handle));
        EXPECT_GT(Peak(std::span(samples).last(Frames(0.01f) * 2)), 0.0f);
    }

    TEST_F(EngineSoundManagerTests, PlayForStopsAfterItsDuration) {
        const SoundHandle handle = sounds.PlayFor("music", 0.1f);

        static_cast<void>(Mix(0.2f));

        EXPECT_FALSE(sounds.IsPlaying(handle));
    }

    TEST_F(EngineSoundManagerTests, PlayForIsSilentAfterItsDuration) {
        sounds.PlayFor("music", 0.1f);

        const auto samples = Mix(0.2f);

        EXPECT_EQ(Peak(std::span(samples).subspan(Frames(0.11f) * 2)), 0.0f);
    }

    TEST_F(EngineSoundManagerTests, PlayForLoopsAShortClipUntilTheDurationEnds) {
        const SoundHandle handle = sounds.PlayFor("click", 0.3f, true);

        static_cast<void>(Mix(0.2f));

        EXPECT_TRUE(sounds.IsPlaying(handle));
    }

    TEST_F(EngineSoundManagerTests, PlayForWithoutLoopEndsWithAShortClip) {
        const SoundHandle handle = sounds.PlayFor("click", 0.3f, false);

        static_cast<void>(Mix(0.2f));

        EXPECT_FALSE(sounds.IsPlaying(handle));
    }

    TEST_F(EngineSoundManagerTests, APausedSoundIsSilent) {
        const SoundHandle handle = sounds.Play("music");

        sounds.Pause(handle);

        EXPECT_EQ(Peak(Mix(0.05f)), 0.0f);
    }

    TEST_F(EngineSoundManagerTests, APausedSoundIsNotPlaying) {
        const SoundHandle handle = sounds.Play("music");

        sounds.Pause(handle);

        EXPECT_FALSE(sounds.IsPlaying(handle));
    }

    TEST_F(EngineSoundManagerTests, AResumedSoundIsHeardAgain) {
        const SoundHandle handle = sounds.Play("music");
        sounds.Pause(handle);
        static_cast<void>(Mix(0.05f));

        sounds.Resume(handle);

        EXPECT_GT(Peak(Mix(0.05f)), 0.0f);
    }

    TEST_F(EngineSoundManagerTests, AStoppedSoundIsSilent) {
        const SoundHandle handle = sounds.Play("music");
        static_cast<void>(Mix(0.01f));

        sounds.Stop(handle);

        EXPECT_EQ(Peak(Mix(0.05f)), 0.0f);
    }

    TEST_F(EngineSoundManagerTests, AnOldHandleDoesNotAffectANewSound) {
        const SoundHandle old = sounds.Play("music");
        sounds.Stop(old);
        const SoundHandle current = sounds.Play("music");

        sounds.Stop(old);

        EXPECT_TRUE(sounds.IsPlaying(current));
    }

    TEST_F(EngineSoundManagerTests, WhenEveryChannelIsBusyTheOldestSoundIsReplaced) {
        const SoundHandle oldest = sounds.Play("music");
        for (std::size_t index = 1; index < EngineSoundManager::MaxSounds; ++index) sounds.Play("music");

        const SoundHandle newest = sounds.Play("music");

        EXPECT_FALSE(sounds.IsPlaying(oldest));
        EXPECT_TRUE(sounds.IsPlaying(newest));
    }

    TEST_F(EngineSoundManagerTests, TheMixNeverLeavesTheValidRange) {
        for (std::size_t index = 0; index < EngineSoundManager::MaxSounds; ++index) sounds.Play("music");

        EXPECT_LE(Peak(Mix(0.05f)), 1.0f);
    }

    TEST_F(EngineSoundManagerTests, ASoundVolumeOfZeroSilencesIt) {
        const SoundHandle handle = sounds.Play("music");

        sounds.SetVolume(handle, 0.0f);

        EXPECT_EQ(Peak(Mix(0.05f)), 0.0f);
    }

    TEST_F(EngineSoundManagerTests, AMasterVolumeOfZeroSilencesEverything) {
        sounds.Play("music");

        sounds.SetMasterVolume(0.0f);

        EXPECT_EQ(Peak(Mix(0.05f)), 0.0f);
    }

    TEST_F(EngineSoundManagerTests, StopAllSilencesEverything) {
        sounds.Play("music");
        sounds.Play("click", true);

        sounds.StopAll();

        EXPECT_FALSE(sounds.IsAnyPlaying());
        EXPECT_EQ(Peak(Mix(0.05f)), 0.0f);
    }

    TEST_F(EngineSoundManagerTests, CleanUpSilencesEverything) {
        sounds.Play("music");
        static_cast<void>(Mix(0.01f));

        sounds.CleanUp();

        EXPECT_FALSE(sounds.IsAnyPlaying());
        EXPECT_EQ(Peak(Mix(0.05f)), 0.0f);
    }

    TEST(EngineSoundManagerWithoutDeviceTests, WithoutAnAudioDeviceSoundsAreIgnored) {
        AssetCache<AudioClip> clips{std::make_unique<const AudioClip>()};
        clips.Add("click", ConstantClip(0.05f));
        EngineSoundManager manager{&clips, AudioOutput::Device};
        SoundManager& sounds = manager;

        const SoundHandle handle = sounds.Play("click");

        EXPECT_FALSE(handle.IsValid());
        EXPECT_FALSE(sounds.Stop(handle));
    }
}
