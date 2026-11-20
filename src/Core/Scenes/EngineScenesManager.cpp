// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "EngineScenesManager.h"
#include <utility>

namespace SpinoCore::Core::Scenes {
    EngineScenesManager::EngineScenesManager(const ScenesManagerInput &input)
    :mStartContext(input.startContext)
    {}

    void EngineScenesManager::PushScene(std::unique_ptr<Scene> scene) {
        mSceneStack.push(std::move(scene));
        mCurrentScene = mSceneStack.top().get();

        mCurrentScene->Start(mStartContext);
    }

    void EngineScenesManager::PopScene() {
        Scene* lastScene = nullptr;

        if (!mSceneStack.empty()) {
            lastScene = mSceneStack.top().get();
            mSceneStack.pop();
        }
        mCurrentScene = lastScene;
    }
}
