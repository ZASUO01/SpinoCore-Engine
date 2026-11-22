// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <gtest/gtest.h>
#include "Core/Actors/EngineActorsManager.h"
#include "Core/Scenes/EngineScenesManager.h"
#include "Core/UI/EngineScreensManager.h"
#include "Graphics/PostProcess/EnginePostProcessManager.h"
#include "Sound/EngineSoundManager.h"
#include "Sound/Synth/EngineSynthManager.h"
#include "SpinoCore/Core/Actors/Actor.h"
#include "SpinoCore/Core/Cameras/Camera.h"
#include "SpinoCore/Core/Cameras/CamerasManager.h"
#include "SpinoCore/Core/Scenes/Contexts.h"
#include "SpinoCore/Core/Input/InputState.h"
#include "SpinoCore/Core/Lighting/GlobalLight.h"
#include "SpinoCore/Core/Scenes/Scene.h"
#include "SpinoCore/Core/UI/Screen.h"
#include "SpinoCore/Core/Window/WindowManager.h"
#include "SpinoCore/Core/Window/WindowProps.h"
#include "SpinoCore/Core/Window/WindowSize.h"
#include "SpinoCore/Files/FileManager.h"
#include "SpinoCore/Graphics/PostProcess.h"
#include "SpinoCore/Sound/Synth/Synthesizer.h"
#include "SpinoCore/Sound/Synth/Timbre.h"
#include "SpinoCore/Utils/Size2D.h"

namespace {
    using SpinoCore::Core::Actors::Actor;
    using SpinoCore::Core::Actors::EngineActorsManager;
    using SpinoCore::Core::Cameras::Camera;
    using SpinoCore::Core::Input::InputState;
    using SpinoCore::Core::Lighting::GlobalLight;
    using SpinoCore::Graphics::EnginePostProcessManager;
    using SpinoCore::Graphics::PostProcess;
    using SpinoCore::Sound::EngineSoundManager;
    using SpinoCore::Sound::Synth::AudioOutput;
    using SpinoCore::Sound::Synth::EngineSynthManager;
    using SpinoCore::Sound::Synth::SynthMode;
    using SpinoCore::Sound::Synth::Timbre;
    using SpinoCore::Core::Cameras::CamerasManager;
    using SpinoCore::Core::Scenes::ControlContext;
    using SpinoCore::Core::Scenes::EngineScenesManager;
    using SpinoCore::Core::Scenes::Scene;
    using SpinoCore::Core::Scenes::ScenesManagerInput;
    using SpinoCore::Core::Scenes::StartContext;
    using SpinoCore::Core::UI::EngineScreensManager;
    using SpinoCore::Core::UI::Screen;
    using SpinoCore::Core::Window::WINDOW_SIZE_MIN;
    using SpinoCore::Core::Window::WindowManager;
    using SpinoCore::Core::Window::WindowMode;
    using SpinoCore::Files::FileManager;
    using SpinoCore::Utils::Size2D;

    class LoadingFileManager final : public FileManager {
    public:
        void LoadFromManifest(const std::filesystem::path&) override { mIsReady = false; }
        void LoadFromSinglePath(SpinoCore::Files::AssetType, std::string, const std::filesystem::path&) override { mIsReady = false; }
        void WriteToSinglePath(const std::filesystem::path&, std::vector<std::uint8_t>) override {}
        [[nodiscard]] bool FileExists(const std::filesystem::path&) const override { return false; }
        [[nodiscard]] bool IsReady() const override { return mIsReady; }
        [[nodiscard]] const std::vector<std::string>& GetFailedLoads() const override { return mFailedLoads; }

        void FinishLoading() { mIsReady = true; }

    private:
        bool mIsReady{true};
        std::vector<std::string> mFailedLoads{};
    };

    class CountingCamerasManager final : public CamerasManager {
    public:
        void AddCamera(std::unique_ptr<Camera>, const std::string&) override {}
        [[nodiscard]] Camera* GetCamera(const std::string&) override { return nullptr; }
        void SetCurrentCamera(const std::string&) override {}
        void RemoveCamera(const std::string&) override {}
        void CleanUp() override { ++cleanUps; }

        int cleanUps{0};
    };

    class FixedWindowManager final : public WindowManager {
    public:
        [[nodiscard]] WindowMode GetMode() const noexcept override { return WindowMode::Windowed; }
        bool SetMode(WindowMode) noexcept override { return false; }
        [[nodiscard]] Size2D GetSize() const noexcept override { return WINDOW_SIZE_MIN; }
        bool SetSize(Size2D) noexcept override { return false; }
    };

    class IdleActor final : public Actor {
    protected:
        void OnUpdate(float) override {}
        void OnProcessInput(const InputState&) override {}
    };

    class SilentTimbre final : public Timbre {
    public:
        void SetupInstrument() override {}
    };

    class LoggedScreen final : public Screen {
    public:
        explicit LoggedScreen(std::vector<std::string>& log) : mLog(log) {}

        void OnEnter() override { mLog.emplace_back("screen entered"); }
        void OnPause() override {}
        void OnResume() override {}
        void OnExit() override { mLog.emplace_back("screen exited"); }

    private:
        std::vector<std::string>& mLog;
    };

    class TrackedScene final : public Scene {
    public:
        TrackedScene(std::vector<std::string>& log, std::string name) : mLog(log), mName(std::move(name)) {}
        ~TrackedScene() override { mLog.emplace_back(mName + " destroyed"); }

        void OnStart(const StartContext& ctx) override {
            mLog.emplace_back(mName + " started");
            ctx.fileManager.LoadFromManifest("manifest.json");
        }

        void OnSetup(const ControlContext& ctx) override {
            mLog.emplace_back(mName + " set up");
            ctx.screensManager.PushScreen(std::make_unique<LoggedScreen>(mLog));
        }

        void OnUpdate(const ControlContext&) override { mLog.emplace_back(mName + " updated"); }
        void OnFinish(const ControlContext&) override { mLog.emplace_back(mName + " finished"); }

    private:
        std::vector<std::string>& mLog;
        std::string mName;
    };

    class EngineScenesManagerTests : public testing::Test {
    protected:
        std::vector<std::string> log{};
        LoadingFileManager fileManager{};
        CountingCamerasManager camerasManager{};
        EngineScreensManager screensManager{};
        FixedWindowManager windowManager{};
        EngineActorsManager actorsManager{};
        GlobalLight globalLight{};
        EnginePostProcessManager postProcessManager{};
        EngineSynthManager synthManager{AudioOutput::Offline};
        EngineSoundManager soundManager{nullptr, AudioOutput::Offline};
        InputState simulationInput{};
        StartContext startContext{.fileManager = fileManager};
        ControlContext controlContext{
            .fileManager = fileManager,
            .camerasManager = camerasManager,
            .screensManager = screensManager,
            .windowManager = windowManager,
            .actorsManager = actorsManager,
            .globalLight = globalLight,
            .postProcessManager = postProcessManager,
            .synthManager = synthManager,
            .soundManager = soundManager,
            .simulationInput = simulationInput
        };
        std::optional<EngineScenesManager> scenesManager{ScenesManagerInput{.startContext = startContext, .controlContext = controlContext}};
    };

    TEST_F(EngineScenesManagerTests, PushingASceneStartsIt) {
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "menu"));

        EXPECT_EQ(log, (std::vector<std::string>{"menu started"}));
    }

    TEST_F(EngineScenesManagerTests, PushedSceneBecomesTheCurrentScene) {
        auto scene = std::make_unique<TrackedScene>(log, "menu");
        const Scene* pushed = scene.get();

        scenesManager->PushScene(std::move(scene));

        EXPECT_EQ(scenesManager->GetCurrentScene(), pushed);
    }

    TEST_F(EngineScenesManagerTests, PushingANullSceneKeepsTheCurrentScene) {
        auto scene = std::make_unique<TrackedScene>(log, "menu");
        const Scene* pushed = scene.get();
        scenesManager->PushScene(std::move(scene));

        scenesManager->PushScene(nullptr);

        EXPECT_EQ(scenesManager->GetCurrentScene(), pushed);
    }

    TEST_F(EngineScenesManagerTests, SceneIsNotSetUpWhileItsResourcesAreLoading) {
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "menu"));

        scenesManager->PrepareCurrentScene();

        EXPECT_FALSE(scenesManager->IsCurrentSceneReady());
    }

    TEST_F(EngineScenesManagerTests, SceneScreensAreNotCreatedWhileItsResourcesAreLoading) {
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "menu"));

        scenesManager->PrepareCurrentScene();

        EXPECT_EQ(log, (std::vector<std::string>{"menu started"}));
    }

    TEST_F(EngineScenesManagerTests, SceneScreensAreCreatedOnceItsResourcesFinishLoading) {
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "menu"));
        scenesManager->PrepareCurrentScene();
        fileManager.FinishLoading();

        scenesManager->PrepareCurrentScene();

        EXPECT_EQ(log, (std::vector<std::string>{"menu started", "menu set up", "screen entered"}));
    }

    TEST_F(EngineScenesManagerTests, ReadySceneIsSetUpOnlyOnce) {
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "menu"));
        fileManager.FinishLoading();

        scenesManager->PrepareCurrentScene();
        scenesManager->PrepareCurrentScene();

        EXPECT_EQ(log, (std::vector<std::string>{"menu started", "menu set up", "screen entered"}));
    }

    TEST_F(EngineScenesManagerTests, SceneIsNotUpdatedBeforeBeingSetUp) {
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "menu"));

        scenesManager->UpdateCurrentScene();

        EXPECT_EQ(log, (std::vector<std::string>{"menu started"}));
    }

    TEST_F(EngineScenesManagerTests, ReadySceneIsUpdated) {
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "menu"));
        fileManager.FinishLoading();
        scenesManager->PrepareCurrentScene();
        log.clear();

        scenesManager->UpdateCurrentScene();

        EXPECT_EQ(log, (std::vector<std::string>{"menu updated"}));
    }

    TEST_F(EngineScenesManagerTests, PushingASceneFinishesThePreviousOneAndRemovesItsScreens) {
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "menu"));
        fileManager.FinishLoading();
        scenesManager->PrepareCurrentScene();
        log.clear();

        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "level"));

        EXPECT_EQ(log, (std::vector<std::string>{"menu finished", "screen exited", "level started"}));
    }

    TEST_F(EngineScenesManagerTests, PushingASceneCleansUpTheCamerasOfThePreviousOne) {
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "menu"));

        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "level"));

        EXPECT_EQ(camerasManager.cleanUps, 1);
    }

    TEST_F(EngineScenesManagerTests, PushingASceneRemovesTheActorsOfThePreviousOne) {
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "menu"));
        actorsManager.AddActor(std::make_unique<IdleActor>());

        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "level"));

        EXPECT_TRUE(actorsManager.GetActors().empty());
    }

    TEST_F(EngineScenesManagerTests, PushingASceneRemovesThePostProcessesOfThePreviousOne) {
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "menu"));
        postProcessManager.AddPostProcess("Flash", PostProcess{.fragmentShader = "void main() { outColor = vec4(1.0); }", .uniforms = {}});

        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "level"));

        EXPECT_FALSE(postProcessManager.Contains("Flash"));
    }

    TEST_F(EngineScenesManagerTests, PushingASceneRemovesTheInstrumentsOfThePreviousOne) {
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "menu"));
        synthManager.Register("lead", Timbre::CreateUnique<SilentTimbre>(), SynthMode::Monophonic);

        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "level"));

        EXPECT_FALSE(synthManager.Contains("lead"));
    }

    TEST_F(EngineScenesManagerTests, SceneThatWasNeverSetUpIsNotFinished) {
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "menu"));
        log.clear();

        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "level"));

        EXPECT_EQ(log, (std::vector<std::string>{"level started"}));
    }

    TEST_F(EngineScenesManagerTests, PoppingFinishesAndDestroysTheTopScene) {
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "menu"));
        fileManager.FinishLoading();
        scenesManager->PrepareCurrentScene();
        log.clear();

        scenesManager->PopScene();

        EXPECT_EQ(log, (std::vector<std::string>{"menu finished", "screen exited", "menu destroyed"}));
    }

    TEST_F(EngineScenesManagerTests, PoppingCleansUpTheCamerasOfTheTopScene) {
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "menu"));

        scenesManager->PopScene();

        EXPECT_EQ(camerasManager.cleanUps, 1);
    }

    TEST_F(EngineScenesManagerTests, AfterPoppingTheSceneBelowBecomesCurrent) {
        auto bottom = std::make_unique<TrackedScene>(log, "menu");
        const Scene* bottomScene = bottom.get();
        scenesManager->PushScene(std::move(bottom));
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "level"));

        scenesManager->PopScene();

        EXPECT_EQ(scenesManager->GetCurrentScene(), bottomScene);
    }

    TEST_F(EngineScenesManagerTests, SceneBelowIsSetUpAgainAfterThePop) {
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "menu"));
        fileManager.FinishLoading();
        scenesManager->PrepareCurrentScene();
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "level"));
        fileManager.FinishLoading();
        scenesManager->PrepareCurrentScene();
        scenesManager->PopScene();
        log.clear();

        scenesManager->PrepareCurrentScene();

        EXPECT_EQ(log, (std::vector<std::string>{"menu set up", "screen entered"}));
    }

    TEST_F(EngineScenesManagerTests, PoppingTheLastSceneLeavesNoCurrentScene) {
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "menu"));

        scenesManager->PopScene();

        EXPECT_EQ(scenesManager->GetCurrentScene(), nullptr);
    }

    TEST_F(EngineScenesManagerTests, PoppingAnEmptyStackDoesNothing) {
        scenesManager->PopScene();

        EXPECT_EQ(scenesManager->GetCurrentScene(), nullptr);
    }

    TEST_F(EngineScenesManagerTests, DestroyingTheManagerFinishesTheCurrentScene) {
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "menu"));
        fileManager.FinishLoading();
        scenesManager->PrepareCurrentScene();
        log.clear();

        scenesManager.reset();

        EXPECT_EQ(log, (std::vector<std::string>{"menu finished", "screen exited", "menu destroyed"}));
    }

    TEST_F(EngineScenesManagerTests, DestroyingTheManagerDestroysScenesFromTopToBottom) {
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "menu"));
        scenesManager->PushScene(std::make_unique<TrackedScene>(log, "level"));
        log.clear();

        scenesManager.reset();

        EXPECT_EQ(log, (std::vector<std::string>{"level destroyed", "menu destroyed"}));
    }
}
