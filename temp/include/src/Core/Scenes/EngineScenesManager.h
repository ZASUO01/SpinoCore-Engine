// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <queue>
#include "SpinoCore/Core/Scenes/ScenesManager.h"

namespace SpinoCore::Core::Scenes {
    struct ScenesManagerInput final {
        StartContext& startCtx;
    };

    class EngineScenesManager final : public ScenesManager {
    public:
        explicit EngineScenesManager(const ScenesManagerInput& input);
        ~EngineScenesManager() override = default;

        EngineScenesManager(const EngineScenesManager&) = delete;
        EngineScenesManager& operator=(const EngineScenesManager&) = delete;
        EngineScenesManager(EngineScenesManager&&) = delete;
        EngineScenesManager& operator=(EngineScenesManager&&) = delete;

        void PushScene(std::unique_ptr<Scene> scene) override;
        void PopScene() override;
    private:
        std::queue<std::unique_ptr<Scene>> mScenes;
        Scene* mCurrentScene{nullptr};

        StartContext& mStartCtx;
    };
}
