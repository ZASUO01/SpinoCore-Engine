// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include <vector>
#include "SpinoCore/Core/UI/Screen.h"
#include "SpinoCore/Core/UI/ScreensManager.h"

namespace SpinoCore::Core::Input {
    class InputState;
}

namespace SpinoCore::Core::Rendering {
    struct UIDrawList;
}

namespace SpinoCore::Core::UI {
    class EngineScreensManager final : public ScreensManager {
    public:
        EngineScreensManager();
        ~EngineScreensManager() override;

        EngineScreensManager(const EngineScreensManager&) = delete;
        EngineScreensManager& operator=(const EngineScreensManager&) = delete;
        EngineScreensManager(EngineScreensManager&&) = delete;
        EngineScreensManager& operator=(EngineScreensManager&&) = delete;

        void PushScreen(std::unique_ptr<Screen> screen) override;
        void PopScreen() override;
        void CleanUp() override;
        void Clear() noexcept;

        void ProcessInputs(const Input::InputState& state) const;
        void GetDrawCommands(Rendering::UIDrawList& drawList) const;
    private:
        std::vector<std::unique_ptr<Screen>> mScreens;
        Screen* mCurrentScreen{nullptr};
    };
}
