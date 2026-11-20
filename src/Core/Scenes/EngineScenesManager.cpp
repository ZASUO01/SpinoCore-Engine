// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "EngineScenesManager.h"
#include <utility>

namespace SpinoCore::Core::Scenes {
    EngineScenesManager::EngineScenesManager(const ScenesManagerInput &input)
    :mStartContext(input.startContext)
    {}

    EngineScenesManager::~EngineScenesManager() {
        // std::stack destroys its elements from the bottom up; scenes on top may depend on
        // the ones below them, so they are destroyed first.
        while (!mSceneStack.empty()) mSceneStack.pop();
    }

    void EngineScenesManager::PushScene(std::unique_ptr<Scene> scene) {
        mSceneStack.push(std::move(scene));
        mCurrentScene = mSceneStack.top().get();

        mCurrentScene->Start(mStartContext);
    }

    void EngineScenesManager::PopScene() {
        if (mSceneStack.empty()) return;

        mSceneStack.pop();
        mCurrentScene = mSceneStack.empty() ? nullptr : mSceneStack.top().get();
    }

    Scene* EngineScenesManager::GetCurrentScene() const noexcept {
        return mCurrentScene;
    }
}
