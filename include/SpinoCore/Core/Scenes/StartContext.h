// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "SpinoCore/Core/UI/ScreensManager.h"
#include "SpinoCore/Core/Cameras/CamerasManager.h"

namespace SpinoCore::Core::Scenes {
    struct StartContext final {
        Cameras::CamerasManager& camerasManager;
        UI::ScreensManager& screensManager;
    };
}
