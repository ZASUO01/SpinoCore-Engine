// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <memory>
#include <utility>
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
        int destructions{0};
    };

    class TrackedScene final : public Scene {
    public:
        explicit TrackedScene(SceneEvents& events) : mEvents(events) {}
        ~TrackedScene() override { ++mEvents.destructions; }

    protected:
        void OnStart(const StartContext& /*ctx*/) override { ++mEvents.starts; }

    private:
        SceneEvents& mEvents;
    };

    class EngineScenesManagerTests : public testing::Test {
    protected:
        StartContext startContext{};
        EngineScenesManager scenesManager{ScenesManagerInput{.startContext = startContext}};
    };

    TEST_F(EngineScenesManagerTests, PushingASceneStartsIt) {
        SceneEvents events;

        scenesManager.PushScene(std::make_unique<TrackedScene>(events));

        EXPECT_EQ(events.starts, 1);
    }

    TEST_F(EngineScenesManagerTests, PushedSceneBecomesTheCurrentScene) {
        SceneEvents events;
        auto scene = std::make_unique<TrackedScene>(events);
        const Scene* pushed = scene.get();

        scenesManager.PushScene(std::move(scene));

        EXPECT_EQ(scenesManager.GetCurrentScene(), pushed);
    }

    TEST_F(EngineScenesManagerTests, PoppingDestroysTheTopScene) {
        SceneEvents events;
        scenesManager.PushScene(std::make_unique<TrackedScene>(events));

        scenesManager.PopScene();

        EXPECT_EQ(events.destructions, 1);
    }

    TEST_F(EngineScenesManagerTests, AfterPoppingTheSceneBelowBecomesCurrent) {
        SceneEvents events;
        auto bottom = std::make_unique<TrackedScene>(events);
        const Scene* bottomScene = bottom.get();
        scenesManager.PushScene(std::move(bottom));
        scenesManager.PushScene(std::make_unique<TrackedScene>(events));

        scenesManager.PopScene();

        EXPECT_EQ(scenesManager.GetCurrentScene(), bottomScene);
    }

    TEST_F(EngineScenesManagerTests, PoppingTheLastSceneLeavesNoCurrentScene) {
        SceneEvents events;
        scenesManager.PushScene(std::make_unique<TrackedScene>(events));

        scenesManager.PopScene();

        EXPECT_EQ(scenesManager.GetCurrentScene(), nullptr);
    }

    TEST_F(EngineScenesManagerTests, PoppingAnEmptyStackDoesNothing) {
        scenesManager.PopScene();

        EXPECT_EQ(scenesManager.GetCurrentScene(), nullptr);
    }
}
