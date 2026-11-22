// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <optional>
#include <gtest/gtest.h>
#include <SDL3/SDL_hints.h>
#include "SDL/Context.h"
#include "Sound/Synth/EngineSynthManager.h"
#include "SpinoCore/Game/GameInfo.h"

namespace {
    using SpinoCore::Game::GameInfo;
    using SpinoCore::SDL::Context;
    using SpinoCore::Sound::Synth::AudioOutput;
    using SpinoCore::Sound::Synth::EngineSynthManager;

    class EngineSynthManagerDeviceTests : public testing::Test {
    protected:
        void SetUp() override {
            SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen");
            SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
            context.emplace(GameInfo{});
        }

        std::optional<Context> context{};
    };

    TEST_F(EngineSynthManagerDeviceTests, TheAudioStreamOpensOnTheDevice) {
        const EngineSynthManager manager{AudioOutput::Device};

        EXPECT_TRUE(manager.HasDevice());
    }

    TEST_F(EngineSynthManagerDeviceTests, TheSampleRateComesFromTheDevice) {
        const EngineSynthManager manager{AudioOutput::Device};

        EXPECT_GT(manager.GetSampleRate(), 0);
    }
}
