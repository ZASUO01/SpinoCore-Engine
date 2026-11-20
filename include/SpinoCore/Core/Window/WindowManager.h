// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "WindowProps.h"
#include "WindowSize.h"

namespace SpinoCore::Core::Window {
    class WindowManager {
    public:
        virtual ~WindowManager() = default;
        [[nodiscard]] virtual WindowMode GetMode() const noexcept = 0;
        virtual bool SetMode(WindowMode mode) noexcept = 0;
        [[nodiscard]] virtual Size2D GetSize() const noexcept = 0;
        virtual bool SetSize(Size2D size) noexcept = 0;
    };
}