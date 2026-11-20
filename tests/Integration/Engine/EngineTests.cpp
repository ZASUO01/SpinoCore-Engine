// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include <SDL3/SDL.h>
#include "Core/Engine/Engine.h"
#include "SpinoCore/Core/Scenes/Scene.h"
#include "SpinoCore/Core/Scenes/StartContext.h"
#include "SpinoCore/Game/GameApplication.h"
#include "SpinoCore/Game/InitializationContext.h"
#include "SpinoCore/LaunchOptions.h"

namespace {
    using SpinoCore::LaunchOptions;
    using SpinoCore::Core::Engine::Engine;
    using SpinoCore::Core::Scenes::Scene;
    using SpinoCore::Core::Scenes::StartContext;
    using SpinoCore::Game::GameApplication;
    using SpinoCore::Game::InitializationContext;

    class TrackedScene final : public Scene {
    public:
        explicit TrackedScene(std::vector<std::string>& destructions) : mDestructions(destructions) {}
        ~TrackedScene() override { mDestructions.emplace_back("scene"); }

    protected:
        void OnStart(const StartContext& /*ctx*/) override {}

    private:
        std::vector<std::string>& mDestructions;
    };

    class TrackedGame final : public GameApplication {
    public:
        explicit TrackedGame(std::vector<std::string>& destructions) : mDestructions(destructions) {}
        ~TrackedGame() override { mDestructions.emplace_back("game"); }

        void Initialize(const InitializationContext& ctx) override {
            ctx.scenesManager.PushScene(std::make_unique<TrackedScene>(mDestructions));
        }

    private:
        std::vector<std::string>& mDestructions;
    };

    class FailingGame final : public GameApplication {
    public:
        void Initialize(const InitializationContext& /*ctx*/) override {
            throw std::runtime_error("game failed to initialize");
        }
    };

    class EngineTests : public testing::Test {
    protected:
        void SetUp() override {
            SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen");
            SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
        }
    };

    TEST_F(EngineTests, DestroyingTheEngineShutsSDLDown) {
        std::vector<std::string> destructions;
        auto engine = std::make_unique<Engine>(std::make_unique<TrackedGame>(destructions), LaunchOptions{});

        engine.reset();

        EXPECT_EQ(SDL_WasInit(0), 0u);
    }

    TEST_F(EngineTests, ScenesAreDestroyedBeforeTheGame) {
        std::vector<std::string> destructions;
        auto engine = std::make_unique<Engine>(std::make_unique<TrackedGame>(destructions), LaunchOptions{});

        engine.reset();

        EXPECT_EQ(destructions, (std::vector<std::string>{"scene", "game"}));
    }

    TEST_F(EngineTests, FailingToInitializeShutsSDLDown) {
        EXPECT_THROW(Engine(std::make_unique<FailingGame>(), LaunchOptions{}), std::runtime_error);

        EXPECT_EQ(SDL_WasInit(0), 0u);
    }
}
