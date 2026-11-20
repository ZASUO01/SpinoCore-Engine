// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once

namespace SpinoCore::Game {
    struct GameInfo;
}

namespace SpinoCore::SDL {
    class Context final {
    public:
        explicit Context(const Game::GameInfo& gameInfo);
        ~Context();

        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;
        Context(Context&&) = delete;
        Context& operator=(Context&&) = delete;
    };
}