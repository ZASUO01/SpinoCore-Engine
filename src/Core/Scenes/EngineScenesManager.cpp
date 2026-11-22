// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "EngineScenesManager.h"
#include <utility>
#include "SpinoCore/Core/Actors/ActorsManager.h"
#include "SpinoCore/Core/Cameras/CamerasManager.h"
#include "SpinoCore/Core/Scenes/Scene.h"
#include "SpinoCore/Core/UI/ScreensManager.h"
#include "SpinoCore/Files/FileManager.h"
#include "SpinoCore/Graphics/PostProcessManager.h"

namespace SpinoCore::Core::Scenes {
    EngineScenesManager::EngineScenesManager(const ScenesManagerInput &input)
    :mStartContext(input.startContext)
    ,mControlContext(input.controlContext)
    {}

    EngineScenesManager::~EngineScenesManager() {
        FinishCurrentScene();
        while (!mSceneStack.empty()) mSceneStack.pop();
    }

    void EngineScenesManager::PushScene(std::unique_ptr<Scene> scene) {
        if (!scene) return;

        FinishCurrentScene();

        mSceneStack.push(std::move(scene));
        mCurrentScene = mSceneStack.top().get();
        mCurrentScene->OnStart(mStartContext);
    }

    void EngineScenesManager::PopScene() {
        if (mSceneStack.empty()) return;

        FinishCurrentScene();

        mSceneStack.pop();
        mCurrentScene = mSceneStack.empty() ? nullptr : mSceneStack.top().get();
    }

    void EngineScenesManager::PrepareCurrentScene() {
        if (!mCurrentScene || mIsCurrentSceneReady || !mStartContext.fileManager.IsReady()) return;

        mIsCurrentSceneReady = true;
        mCurrentScene->OnSetup(mControlContext);
    }

    void EngineScenesManager::UpdateCurrentScene() {
        if (mCurrentScene && mIsCurrentSceneReady) mCurrentScene->OnUpdate(mControlContext);
    }

    Scene* EngineScenesManager::GetCurrentScene() const noexcept {
        return mCurrentScene;
    }

    bool EngineScenesManager::IsCurrentSceneReady() const noexcept {
        return mIsCurrentSceneReady;
    }

    void EngineScenesManager::FinishCurrentScene() {
        if (!mCurrentScene) return;

        if (mIsCurrentSceneReady) {
            mIsCurrentSceneReady = false;
            mCurrentScene->OnFinish(mControlContext);
        }

        mControlContext.screensManager.CleanUp();
        mControlContext.camerasManager.CleanUp();
        mControlContext.actorsManager.CleanUp();
        mControlContext.postProcessManager.CleanUp();
    }
}
