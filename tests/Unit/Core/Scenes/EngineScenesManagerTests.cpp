// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <memory>
#include <optional>
#include <utility>
#include <vector>
#include <gtest/gtest.h>
#include "Core/Scenes/EngineScenesManager.h"
#include "SpinoCore/Core/Scenes/Scene.h"
#include "SpinoCore/Core/Scenes/StartContext.h"

namespace {
    using SpinoCore::Core::Scenes::EngineScenesManager;
    using SpinoCore::Core::Scenes::Scene;
    using SpinoCore::Core::Scenes::ScenesManagerInput;
    using SpinoCore::Core::Scenes::StartContext;

    struct SceneEvents final {
        int starts{0};
        std::vector<int> destroyedScenes{};
    };

    class TrackedScene final : public Scene {
    public:
        explicit TrackedScene(SceneEvents& events, const int id = 0) : mEvents(events), mId(id) {}
        ~TrackedScene() override { mEvents.destroyedScenes.push_back(mId); }

    protected:
        void OnStart(const StartContext& /*ctx*/) override { ++mEvents.starts; }

    private:
        SceneEvents& mEvents;
        int mId;
    };

    class EngineScenesManagerTests : public testing::Test {
    protected:
        // Declared before the manager so it outlives the scenes the manager destroys.
        SceneEvents events{};
        StartContext startContext{};
        EngineScenesManager scenesManager{ScenesManagerInput{.startContext = startContext}};
    };

    TEST_F(EngineScenesManagerTests, PushingASceneStartsIt) {
        scenesManager.PushScene(std::make_unique<TrackedScene>(events));

        EXPECT_EQ(events.starts, 1);
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
