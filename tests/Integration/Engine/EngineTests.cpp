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
#include "SpinoCore/Core/UI/Screen.h"
#include "SpinoCore/Core/UI/ScreensManager.h"
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

    class LoggedScreen final : public SpinoCore::Core::UI::Screen {
    public:
        explicit LoggedScreen(std::vector<std::string>& log) : mLog(log) {}
        ~LoggedScreen() override { mLog.emplace_back("screen destroyed"); }

        void OnEnter() override { mLog.emplace_back("screen entered"); }
        void OnPause() override {}
        void OnResume() override {}
        void OnExit() override {}

    private:
        std::vector<std::string>& mLog;
    };

    class ScreenPushingScene final : public Scene {
    public:
        explicit ScreenPushingScene(std::vector<std::string>& log) : mLog(log) {}

    protected:
        void OnStart(const StartContext& ctx) override {
            ctx.screensManager.PushScreen(std::make_unique<LoggedScreen>(mLog));
        }

    private:
        std::vector<std::string>& mLog;
    };

    class ScreenGame final : public GameApplication {
    public:
        explicit ScreenGame(std::vector<std::string>& log) : mLog(log) {}

        void Initialize(const InitializationContext& ctx) override {
            ctx.scenesManager.PushScene(std::make_unique<ScreenPushingScene>(mLog));
        }

    private:
        std::vector<std::string>& mLog;
    };

    // Records whether the scene could reach the engine's default camera when it started.
    class CameraProbeScene final : public Scene {
    public:
        explicit CameraProbeScene(bool& sawDefaultCamera) : mSawDefaultCamera(sawDefaultCamera) {}

    protected:
        void OnStart(const StartContext& ctx) override {
            mSawDefaultCamera = ctx.camerasManager.GetCamera("default") != nullptr;
        }

    private:
        bool& mSawDefaultCamera;
    };

    class CameraProbeGame final : public GameApplication {
    public:
        explicit CameraProbeGame(bool& sawDefaultCamera) : mSawDefaultCamera(sawDefaultCamera) {}

        void Initialize(const InitializationContext& ctx) override {
            ctx.scenesManager.PushScene(std::make_unique<CameraProbeScene>(mSawDefaultCamera));
        }

    private:
        bool& mSawDefaultCamera;
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

    TEST_F(EngineTests, ScenesStartWithTheEngineCamerasManager) {
        bool sawDefaultCamera = false;

        { const Engine engine{std::make_unique<CameraProbeGame>(sawDefaultCamera), LaunchOptions{}}; }

        EXPECT_TRUE(sawDefaultCamera);
    }

    TEST_F(EngineTests, ScenesCanPushScreensThatLiveUntilTheEngineShutsDown) {
        std::vector<std::string> log;

        { const Engine engine{std::make_unique<ScreenGame>(log), LaunchOptions{}}; }

        EXPECT_EQ(log, (std::vector<std::string>{"screen entered", "screen destroyed"}));
    }

    TEST_F(EngineTests, FailingToInitializeShutsSDLDown) {
        EXPECT_THROW(Engine(std::make_unique<FailingGame>(), LaunchOptions{}), std::runtime_error);

        EXPECT_EQ(SDL_WasInit(0), 0u);
    }
}
