// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <gtest/gtest.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_hints.h>
#include "SpinoCore/Game/GameApplication.h"
#include "SpinoCore/Game/InitializationContext.h"
#include "SpinoCore/Launch.h"
#include "SpinoCore/Logs/LogLevel.h"
#include "SpinoCore/Logs/LogOptions.h"

namespace {
    using SpinoCore::Launch;
    using SpinoCore::Game::GameApplication;
    using SpinoCore::Game::InitializationContext;
    using SpinoCore::Logs::Level;
    using SpinoCore::Logs::LogOptions;

    class QuittingGame final : public GameApplication {
    public:
        void Initialize(const InitializationContext&) override {
            SDL_Event quit{};
            quit.type = SDL_EVENT_QUIT;
            SDL_PushEvent(&quit);
        }
    };

    class FailingGame final : public GameApplication {
    public:
        void Initialize(const InitializationContext&) override {
            throw std::runtime_error("game failed to initialize");
        }
    };

    class LaunchTests : public testing::Test {
    protected:
        void SetUp() override {
            SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen");
            SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
        }
    };

    TEST_F(LaunchTests, LaunchingWithoutAGameFails) {
        const int exitCode = Launch(nullptr, {}, LogOptions{.level = Level::None});

        EXPECT_EQ(exitCode, EXIT_FAILURE);
    }

    TEST_F(LaunchTests, LaunchingAGameThatThrowsOnInitializeFails) {
        const int exitCode = Launch(std::make_unique<FailingGame>(), {}, LogOptions{.level = Level::None});

        EXPECT_EQ(exitCode, EXIT_FAILURE);
    }

    TEST_F(LaunchTests, LaunchingAGameRunsUntilQuitIsRequested) {
        const int exitCode = Launch(std::make_unique<QuittingGame>(), {}, LogOptions{.level = Level::None});

        EXPECT_EQ(exitCode, EXIT_SUCCESS);
    }
}
