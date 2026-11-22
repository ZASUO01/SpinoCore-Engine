// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include <stack>
#include "SpinoCore/Core/Scenes/Contexts.h"
#include "SpinoCore/Core/Scenes/ScenesManager.h"

namespace SpinoCore::Core::Scenes {
    class Scene;

    struct ScenesManagerInput final {
        const StartContext& startContext;
        const ControlContext& controlContext;
    };

    class EngineScenesManager final : public ScenesManager {
    public:
        explicit EngineScenesManager(const ScenesManagerInput& input);
        ~EngineScenesManager() override;

        EngineScenesManager(const EngineScenesManager&) = delete;
        EngineScenesManager& operator=(const EngineScenesManager&) = delete;
        EngineScenesManager(EngineScenesManager&&) = delete;
        EngineScenesManager& operator=(EngineScenesManager&&) = delete;

        void PushScene(std::unique_ptr<Scene> scene) override;
        void PopScene() override;

        void PrepareCurrentScene();
        void UpdateCurrentScene();

        [[nodiscard]] Scene* GetCurrentScene() const noexcept;
        [[nodiscard]] bool IsCurrentSceneReady() const noexcept;

    private:
        void FinishCurrentScene();

        std::stack<std::unique_ptr<Scene>> mSceneStack;
        Scene* mCurrentScene{nullptr};
        bool mIsCurrentSceneReady{false};

        StartContext mStartContext;
        ControlContext mControlContext;
    };
}
