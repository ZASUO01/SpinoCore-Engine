// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_mouse.h>
#include "Core/Engine/Engine.h"
#include "SpinoCore/Core/Cameras/CamerasManager.h"
#include "SpinoCore/Core/Scenes/Contexts.h"
#include "SpinoCore/Core/Scenes/Scene.h"
#include "SpinoCore/Core/Scenes/ScenesManager.h"
#include "SpinoCore/Core/UI/Screen.h"
#include "SpinoCore/Core/UI/ScreensManager.h"
#include "SpinoCore/Files/FileManager.h"
#include "SpinoCore/Game/GameApplication.h"
#include "SpinoCore/Game/InitializationContext.h"
#include "SpinoCore/LaunchOptions.h"

namespace {
    using SpinoCore::LaunchOptions;
    using SpinoCore::Core::Engine::Engine;
    using SpinoCore::Core::Scenes::ControlContext;
    using SpinoCore::Core::Scenes::Scene;
    using SpinoCore::Core::Scenes::StartContext;
    using SpinoCore::Files::AssetType;
    using SpinoCore::Game::GameApplication;
    using SpinoCore::Game::InitializationContext;

    class TrackedScene final : public Scene {
    public:
        explicit TrackedScene(std::vector<std::string>& destructions) : mDestructions(destructions) {}
        ~TrackedScene() override { mDestructions.emplace_back("scene"); }

        void OnStart(const StartContext&) override {}
        void OnSetup(const ControlContext&) override {}
        void OnUpdate(const ControlContext&) override {}
        void OnFinish(const ControlContext&) override {}

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
        void OnExit() override { mLog.emplace_back("screen exited"); }

    private:
        std::vector<std::string>& mLog;
    };

    void RequestQuit() {
        SDL_Event quit{};
        quit.type = SDL_EVENT_QUIT;
        SDL_PushEvent(&quit);
    }

    class ScreenPushingScene final : public Scene {
    public:
        explicit ScreenPushingScene(std::vector<std::string>& log) : mLog(log) {}

        void OnStart(const StartContext&) override { mLog.emplace_back("scene started"); }

        void OnSetup(const ControlContext& ctx) override {
            mLog.emplace_back("scene set up");
            ctx.screensManager.PushScreen(std::make_unique<LoggedScreen>(mLog));
            RequestQuit();
        }

        void OnUpdate(const ControlContext&) override {}
        void OnFinish(const ControlContext&) override { mLog.emplace_back("scene finished"); }

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

    class MissingTextureScene final : public Scene {
    public:
        explicit MissingTextureScene(std::vector<std::string>& failedLoads) : mFailedLoads(failedLoads) {}

        void OnStart(const StartContext& ctx) override {
            ctx.fileManager.LoadFromSinglePath(AssetType::Texture, "ghost", "EngineTests/missing.png");
        }

        void OnSetup(const ControlContext& ctx) override {
            mFailedLoads = ctx.fileManager.GetFailedLoads();
            RequestQuit();
        }

        void OnUpdate(const ControlContext&) override {}
        void OnFinish(const ControlContext&) override {}

    private:
        std::vector<std::string>& mFailedLoads;
    };

    class MissingTextureGame final : public GameApplication {
    public:
        explicit MissingTextureGame(std::vector<std::string>& failedLoads) : mFailedLoads(failedLoads) {}

        void Initialize(const InitializationContext& ctx) override {
            ctx.scenesManager.PushScene(std::make_unique<MissingTextureScene>(mFailedLoads));
        }

    private:
        std::vector<std::string>& mFailedLoads;
    };

    class CameraProbeScene final : public Scene {
    public:
        explicit CameraProbeScene(bool& sawDefaultCamera) : mSawDefaultCamera(sawDefaultCamera) {}

        void OnStart(const StartContext&) override {}

        void OnSetup(const ControlContext& ctx) override {
            mSawDefaultCamera = ctx.camerasManager.GetCamera("default") != nullptr;
            RequestQuit();
        }

        void OnUpdate(const ControlContext&) override {}
        void OnFinish(const ControlContext&) override {}

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

    class FailingAfterPushingGame final : public GameApplication {
    public:
        explicit FailingAfterPushingGame(std::vector<std::string>& destructions) : mDestructions(destructions) {}
        ~FailingAfterPushingGame() override { mDestructions.emplace_back("game"); }

        void Initialize(const InitializationContext& ctx) override {
            ctx.scenesManager.PushScene(std::make_unique<TrackedScene>(mDestructions));
            throw std::runtime_error("game failed after pushing a scene");
        }

    private:
        std::vector<std::string>& mDestructions;
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

    TEST_F(EngineTests, ScenesAreSetUpWithTheEngineCamerasManager) {
        bool sawDefaultCamera = false;
        Engine engine{std::make_unique<CameraProbeGame>(sawDefaultCamera), LaunchOptions{}};

        engine.Run();

        EXPECT_TRUE(sawDefaultCamera);
    }

    TEST_F(EngineTests, SceneScreensAreNotCreatedBeforeTheEngineRuns) {
        std::vector<std::string> log;

        const Engine engine{std::make_unique<ScreenGame>(log), LaunchOptions{}};

        EXPECT_EQ(log, (std::vector<std::string>{"scene started"}));
    }

    TEST_F(EngineTests, RunningTheEngineSetsTheSceneUpOnceItsResourcesAreLoaded) {
        std::vector<std::string> log;
        Engine engine{std::make_unique<ScreenGame>(log), LaunchOptions{}};

        engine.Run();

        EXPECT_EQ(log, (std::vector<std::string>{"scene started", "scene set up", "screen entered"}));
    }

    TEST_F(EngineTests, ShuttingDownFinishesTheSceneAndRemovesItsScreens) {
        std::vector<std::string> log;
        auto engine = std::make_unique<Engine>(std::make_unique<ScreenGame>(log), LaunchOptions{});
        engine->Run();
        log.clear();

        engine.reset();

        EXPECT_EQ(log, (std::vector<std::string>{"scene finished", "screen exited", "screen destroyed"}));
    }

    TEST_F(EngineTests, ScenesAreSetUpKnowingWhichResourcesFailedToLoad) {
        std::vector<std::string> failedLoads;
        Engine engine{std::make_unique<MissingTextureGame>(failedLoads), LaunchOptions{}};

        engine.Run();

        EXPECT_EQ(failedLoads, (std::vector<std::string>{"ghost"}));
    }

    TEST_F(EngineTests, FailingToInitializeShutsSDLDown) {
        EXPECT_THROW(Engine(std::make_unique<FailingGame>(), LaunchOptions{}), std::runtime_error);

        EXPECT_EQ(SDL_WasInit(0), 0u);
    }

    TEST_F(EngineTests, FailingToInitializeStillDestroysScenesBeforeTheGame) {
        std::vector<std::string> destructions;

        EXPECT_THROW(Engine(std::make_unique<FailingAfterPushingGame>(destructions), LaunchOptions{}), std::runtime_error);

        EXPECT_EQ(destructions, (std::vector<std::string>{"scene", "game"}));
        EXPECT_EQ(SDL_WasInit(0), 0u);
    }

    TEST_F(EngineTests, KeepsTheSystemCursorVisibleByDefault) {
        std::vector<std::string> destructions;

        const Engine engine{std::make_unique<TrackedGame>(destructions), LaunchOptions{}};

        EXPECT_TRUE(SDL_CursorVisible());
    }

    TEST_F(EngineTests, HidesTheSystemCursorWhenTheLaunchOptionsAskForIt) {
        std::vector<std::string> destructions;
        LaunchOptions launchOptions{};
        launchOptions.windowProps.hideSystemCursor = true;

        const Engine engine{std::make_unique<TrackedGame>(destructions), launchOptions};

        EXPECT_FALSE(SDL_CursorVisible());
    }
}
