// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once

namespace SpinoCore::App {
    struct AppModules;

    class UserApplication {
    public:
        virtual ~UserApplication() = default;
        virtual void Initialize(AppModules& modules) = 0;
    };
}