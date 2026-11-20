// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "TypeEvent.h"
#include "SpinoCore/Utils/Size2D.h"

namespace SpinoCore::Core::Events {
    struct WindowResizeEvent final : TypeEvent {
        Utils::Size2D size{};
        Utils::Size2D pixelSize{};
    };

    struct WindowVisibilityChangedEvent final : TypeEvent {
        bool visible{true};
    };

    struct WindowPresentationChangedEvent final : TypeEvent {};
}
