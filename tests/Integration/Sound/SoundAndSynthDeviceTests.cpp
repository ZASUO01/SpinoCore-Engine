// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <memory>
#include <optional>
#include <gtest/gtest.h>
#include <SDL3/SDL_hints.h>
#include "Assets/AssetCache.h"
#include "SDL/Context.h"
#include "Sound/EngineSoundManager.h"
#include "Sound/Resources/AudioClip.h"
#include "Sound/Synth/EngineSynthManager.h"
#include "SpinoCore/Game/GameInfo.h"

namespace {
    using SpinoCore::Assets::AssetCache;
    using SpinoCore::Game::GameInfo;
    using SpinoCore::SDL::Context;
    using SpinoCore::Sound::EngineSoundManager;
    using SpinoCore::Sound::Resources::AudioClip;
    using SpinoCore::Sound::Synth::AudioOutput;
    using SpinoCore::Sound::Synth::EngineSynthManager;

    class SoundAndSynthDeviceTests : public testing::Test {
    protected:
        void SetUp() override {
            SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen");
            SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
            context.emplace(GameInfo{});
        }

        std::optional<Context> context{};
        AssetCache<AudioClip> clips{std::make_unique<const AudioClip>()};
    };

    TEST_F(SoundAndSynthDeviceTests, TheSoundStreamOpensOnTheDevice) {
        const EngineSoundManager sounds{&clips, AudioOutput::Device};

        EXPECT_TRUE(sounds.HasDevice());
    }

    TEST_F(SoundAndSynthDeviceTests, SoundsAndTheSynthShareTheSameDevice) {
        const EngineSynthManager synth{AudioOutput::Device};
        const EngineSoundManager sounds{&clips, AudioOutput::Device};

        EXPECT_TRUE(synth.HasDevice());
        EXPECT_TRUE(sounds.HasDevice());
    }
}
