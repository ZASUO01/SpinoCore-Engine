// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <stack>
#include <memory>
#include "SpinoCore/Core/Scenes/Scene.h"
#include "SpinoCore/Core/Scenes/ScenesManager.h"
#include "SpinoCore/Core/Scenes/StartContext.h"

namespace SpinoCore::Core::Scenes {
    struct ScenesManagerInput final {
        const StartContext& startContext;
    };

    class EngineScenesManager final : public ScenesManager {
    public:
        explicit EngineScenesManager(const ScenesManagerInput& input);
        ~EngineScenesManager() override = default;

        void PushScene(std::unique_ptr<Scene> scene) override;
        void PopScene() override;

    private:
        std::stack<std::unique_ptr<Scene>> mSceneStack;
        Scene* mCurrentScene{nullptr};

        StartContext mStartContext;
    };
}
