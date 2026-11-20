// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <optional>
#include <stdexcept>
#include <string>
#include <gtest/gtest.h>
#include <SDL3/SDL.h>
#include "SDL/Context.h"
#include "SpinoCore/Game/GameInfo.h"

namespace {
    using SpinoCore::Game::GameInfo;
    using SpinoCore::SDL::Context;

    class SDLContextTests : public testing::Test {
    protected:
        void SetUp() override {
            SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "dummy");
            SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
        }
    };

    TEST_F(SDLContextTests, CreatingAContextInitializesVideoAndAudio) {
        const GameInfo gameInfo{};
        const Context context{gameInfo};

        EXPECT_EQ(SDL_WasInit(SDL_INIT_VIDEO | SDL_INIT_AUDIO), SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    }

    TEST_F(SDLContextTests, CreatingAContextSetsAppMetadata) {
        GameInfo gameInfo{};
        gameInfo.name = "Spino Test Game";
        gameInfo.version = "2.0.0";
        gameInfo.identifier = "Test identifier";
        gameInfo.creator = "Test creator";
        gameInfo.url = "https://www.spino.com/";

        const Context context{gameInfo};

        EXPECT_EQ(std::string{SDL_GetAppMetadataProperty(SDL_PROP_APP_METADATA_NAME_STRING)}, "Spino Test Game");
        EXPECT_EQ(std::string{SDL_GetAppMetadataProperty(SDL_PROP_APP_METADATA_VERSION_STRING)}, "2.0.0");
        EXPECT_EQ(std::string{SDL_GetAppMetadataProperty(SDL_PROP_APP_METADATA_IDENTIFIER_STRING)}, "Test identifier");
        EXPECT_EQ(std::string{SDL_GetAppMetadataProperty(SDL_PROP_APP_METADATA_CREATOR_STRING)}, "Test creator");
        EXPECT_EQ(std::string{SDL_GetAppMetadataProperty(SDL_PROP_APP_METADATA_URL_STRING)}, "https://www.spino.com/");
    }

    TEST_F(SDLContextTests, DestroyingAContextShutsSDLDown) {
        const GameInfo gameInfo{};
        auto context = std::make_optional<Context>(gameInfo);

        context.reset();

        EXPECT_EQ(SDL_WasInit(0), 0u);
    }

    TEST_F(SDLContextTests, CreatingASecondContextWhileOneIsAliveThrows) {
        const GameInfo gameInfo{};
        const Context firstContext{gameInfo};

        EXPECT_THROW(Context{gameInfo}, std::logic_error);
    }
}
