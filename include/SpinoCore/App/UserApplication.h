// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once

namespace SpinoCore::App {
    class UserApplication {
    public:
        virtual ~UserApplication() = default;
        [[nodiscard]] virtual bool Initialize() = 0;
    };
}