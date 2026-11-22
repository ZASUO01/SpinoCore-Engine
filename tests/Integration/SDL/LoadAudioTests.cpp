// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include "../../Support/AudioBuilder.h"
#include "SDL/Helpers/Helpers.h"

namespace {
    using SpinoCore::SDL::Helpers::LoadAudio;
    using SpinoCore::Tests::SilentMp3;
    using SpinoCore::Tests::SineWav;

    constexpr int EngineRate = 48000;
    constexpr int Stereo = 2;

    TEST(LoadAudioTests, AWavIsDecodedToTheRequestedChannels) {
        const auto wav = SineWav(0.1f);

        const auto audio = LoadAudio(wav, EngineRate, Stereo);

        ASSERT_TRUE(audio.has_value());
        EXPECT_EQ(audio->channels, Stereo);
    }

    TEST(LoadAudioTests, AWavIsResampledToTheRequestedRate) {
        const auto wav = SineWav(0.1f, 22050);

        const auto audio = LoadAudio(wav, EngineRate, Stereo);

        ASSERT_TRUE(audio.has_value());
        EXPECT_EQ(audio->sampleRate, EngineRate);
        EXPECT_NEAR(audio->duration, 0.1f, 0.005f);
    }

    TEST(LoadAudioTests, AWavKeepsItsSignal) {
        const auto wav = SineWav(0.1f);

        const auto audio = LoadAudio(wav, EngineRate, Stereo);

        ASSERT_TRUE(audio.has_value());
        EXPECT_GT(std::ranges::max(audio->samples), 0.3f);
    }

    TEST(LoadAudioTests, AnMp3IsDecoded) {
        const auto mp3 = SilentMp3();

        const auto audio = LoadAudio(mp3, EngineRate, Stereo);

        ASSERT_TRUE(audio.has_value());
        EXPECT_GT(audio->duration, 0.0f);
    }

    TEST(LoadAudioTests, DataThatIsNotAudioIsRejected) {
        const std::string text = "definitely not audio";
        const std::vector<std::uint8_t> bytes(text.begin(), text.end());

        EXPECT_FALSE(LoadAudio(bytes, EngineRate, Stereo).has_value());
    }

    TEST(LoadAudioTests, EmptyDataIsRejected) {
        const std::vector<std::uint8_t> bytes;

        EXPECT_FALSE(LoadAudio(bytes, EngineRate, Stereo).has_value());
    }

    TEST(LoadAudioTests, AFileOnDiskIsDecoded) {
        const auto path = std::filesystem::temp_directory_path() / "spinocore_load_audio_test.wav";
        const auto wav = SineWav(0.05f);
        std::ofstream(path, std::ios::binary).write(reinterpret_cast<const char*>(wav.data()), static_cast<std::streamsize>(wav.size()));

        const auto audio = LoadAudio(path, EngineRate, Stereo);

        std::filesystem::remove(path);
        EXPECT_TRUE(audio.has_value());
    }
}
