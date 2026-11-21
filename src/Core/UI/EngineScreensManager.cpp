// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "EngineScreensManager.h"
#include <utility>

namespace SpinoCore::Core::UI {
    EngineScreensManager::EngineScreensManager() = default;

    EngineScreensManager::~EngineScreensManager() {
        Clear();
    }

    void EngineScreensManager::Clear() noexcept {
        // A vector destroys its elements from the front; screens on top may depend on the ones
        // below them, so they are destroyed first.
        mCurrentScreen = nullptr;
        while (!mScreens.empty()) mScreens.pop_back();
    }

    void EngineScreensManager::PushScreen(std::unique_ptr<Screen> screen) {
        if (mCurrentScreen) {
            mCurrentScreen->OnPause();
        }

        mScreens.emplace_back(std::move(screen));
        mCurrentScreen = mScreens.back().get();
        mCurrentScreen->OnEnter();
    }

    void EngineScreensManager::PopScreen() {
        if (mScreens.empty()) return;

        mScreens.back()->OnExit();
        mScreens.pop_back();

        mCurrentScreen = mScreens.empty() ? nullptr : mScreens.back().get();
        if (mCurrentScreen) mCurrentScreen->OnResume();
    }

    void EngineScreensManager::GetDrawCommands(Rendering::UIDrawList &drawList) const {
        int drawLayer = 1;
        for (const auto& screen : mScreens) {
            if (!screen->IsVisible()) {
                drawLayer++;
                continue;
            }
            screen->GetDrawCommands(Screen::ManagerKey{}, drawList, drawLayer);
            drawLayer++;
        }
    }
}
