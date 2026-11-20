// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "EngineScenesManager.h"

namespace SpinoCore::Core::Scenes {
    EngineScenesManager::EngineScenesManager(const ScenesManagerInput &input)
    :mStartCtx(input.startCtx)
    {}

    void EngineScenesManager::PushScene(std::unique_ptr<Scene> scene) {
        mScenes.push(std::move(scene));
        mCurrentScene = mScenes.back().get();
        mCurrentScene->OnStart(mStartCtx);
    }

    void EngineScenesManager::PopScene() {
        if (!mScenes.empty()) {
            mScenes.pop();
            mCurrentScene = mScenes.back().get();
        }
    }
}
