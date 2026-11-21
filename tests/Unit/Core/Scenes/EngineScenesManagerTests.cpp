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
#include "Core/Scenes/EngineScenesManager.h"
#include "SpinoCore/Core/Cameras/Camera.h"
#include "SpinoCore/Core/Cameras/CamerasManager.h"
#include "SpinoCore/Core/Scenes/Scene.h"
#include "SpinoCore/Core/Scenes/StartContext.h"
#include "SpinoCore/Core/UI/Screen.h"
#include "SpinoCore/Core/UI/ScreensManager.h"
#include "SpinoCore/Files/FileManager.h"

namespace {
    using SpinoCore::Core::Cameras::Camera;
    using SpinoCore::Core::Cameras::CamerasManager;
    using SpinoCore::Core::Scenes::EngineScenesManager;
    using SpinoCore::Core::Scenes::Scene;
    using SpinoCore::Core::Scenes::ScenesManagerInput;
    using SpinoCore::Core::Scenes::StartContext;
    using SpinoCore::Core::UI::Screen;
    using SpinoCore::Core::UI::ScreensManager;
    using SpinoCore::Files::FileManager;

    struct SceneEvents final {
        int starts{0};
        std::vector<int> destroyedScenes{};
        const FileManager* startedWithFiles{nullptr};
        const CamerasManager* startedWithCameras{nullptr};
        const ScreensManager* startedWithScreens{nullptr};
    };

    // The scenes manager only hands this to the scenes; it never calls into it.
    class NullCamerasManager final : public CamerasManager {
    public:
        void AddCamera(std::unique_ptr<Camera> /*camera*/, const std::string& /*name*/) override {}
        [[nodiscard]] Camera* GetCamera(const std::string& /*name*/) override { return nullptr; }
        void SetCurrentCamera(const std::string& /*name*/) override {}
        void RemoveCamera(const std::string& /*name*/) override {}
    };

    class NullFileManager final : public FileManager {
    public:
        void LoadFromManifest(const std::filesystem::path& /*manifestPath*/) override {}
        void LoadFromSinglePath(SpinoCore::Files::AssetType /*type*/, std::string /*id*/, const std::filesystem::path& /*path*/) override {}
        void WriteToSinglePath(const std::filesystem::path& /*path*/, std::vector<std::uint8_t> /*data*/) override {}
        [[nodiscard]] bool FileExists(const std::filesystem::path& /*path*/) const override { return false; }
    };

    class NullScreensManager final : public ScreensManager {
    public:
        void PushScreen(std::unique_ptr<Screen> /*screen*/) override {}
        void PopScreen() override {}
    };

    class TrackedScene final : public Scene {
    public:
        explicit TrackedScene(SceneEvents& events, const int id = 0) : mEvents(events), mId(id) {}
        ~TrackedScene() override { mEvents.destroyedScenes.push_back(mId); }

    protected:
        void OnStart(const StartContext& ctx) override {
            ++mEvents.starts;
            mEvents.startedWithFiles = &ctx.fileManager;
            mEvents.startedWithCameras = &ctx.camerasManager;
            mEvents.startedWithScreens = &ctx.screensManager;
        }

    private:
        SceneEvents& mEvents;
        int mId;
    };

    class EngineScenesManagerTests : public testing::Test {
    protected:
        // Declared before the manager so it outlives the scenes the manager destroys.
        SceneEvents events{};
        NullFileManager fileManager{};
        NullCamerasManager camerasManager{};
        NullScreensManager screensManager{};
        StartContext startContext{.fileManager = fileManager, .camerasManager = camerasManager, .screensManager = screensManager};
        EngineScenesManager scenesManager{ScenesManagerInput{.startContext = startContext}};
    };

    TEST_F(EngineScenesManagerTests, PushingASceneStartsIt) {
        scenesManager.PushScene(std::make_unique<TrackedScene>(events));

        EXPECT_EQ(events.starts, 1);
    }

    TEST_F(EngineScenesManagerTests, StartedScenesReceiveTheEngineManagers) {
        scenesManager.PushScene(std::make_unique<TrackedScene>(events));

        EXPECT_EQ(events.startedWithFiles, &fileManager);
        EXPECT_EQ(events.startedWithCameras, &camerasManager);
        EXPECT_EQ(events.startedWithScreens, &screensManager);
    }

    TEST_F(EngineScenesManagerTests, PushedSceneBecomesTheCurrentScene) {
        auto scene = std::make_unique<TrackedScene>(events);
        const Scene* pushed = scene.get();

        scenesManager.PushScene(std::move(scene));

        EXPECT_EQ(scenesManager.GetCurrentScene(), pushed);
    }

    TEST_F(EngineScenesManagerTests, PoppingDestroysTheTopScene) {
        scenesManager.PushScene(std::make_unique<TrackedScene>(events));

        scenesManager.PopScene();

        EXPECT_EQ(events.destroyedScenes.size(), 1u);
    }

    TEST_F(EngineScenesManagerTests, AfterPoppingTheSceneBelowBecomesCurrent) {
        auto bottom = std::make_unique<TrackedScene>(events);
        const Scene* bottomScene = bottom.get();
        scenesManager.PushScene(std::move(bottom));
        scenesManager.PushScene(std::make_unique<TrackedScene>(events));

        scenesManager.PopScene();

        EXPECT_EQ(scenesManager.GetCurrentScene(), bottomScene);
    }

    TEST_F(EngineScenesManagerTests, PoppingTheLastSceneLeavesNoCurrentScene) {
        scenesManager.PushScene(std::make_unique<TrackedScene>(events));

        scenesManager.PopScene();

        EXPECT_EQ(scenesManager.GetCurrentScene(), nullptr);
    }

    TEST_F(EngineScenesManagerTests, PoppingAnEmptyStackDoesNothing) {
        scenesManager.PopScene();

        EXPECT_EQ(scenesManager.GetCurrentScene(), nullptr);
    }

    TEST_F(EngineScenesManagerTests, DestroyingTheManagerDestroysScenesFromTopToBottom) {
        std::optional<EngineScenesManager> manager{ScenesManagerInput{.startContext = startContext}};
        manager->PushScene(std::make_unique<TrackedScene>(events, 1));
        manager->PushScene(std::make_unique<TrackedScene>(events, 2));

        manager.reset();

        EXPECT_EQ(events.destroyedScenes, (std::vector<int>{2, 1}));
    }
}
