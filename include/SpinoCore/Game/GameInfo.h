// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <string>

namespace SpinoCore::Game {
    /// @brief Application metadata given to SDL.
    struct GameInfo final {
        /// Name of the game.
        std::string name{"SpinoCore App"};
        /// Version of the game.
        std::string version{"0.1.0"};
        /// Unique reverse-domain identifier.
        std::string identifier{"io.github.zasuo01.spinocore"};
        /// Author of the game.
        std::string creator{"ZASUO01"};
        /// Web page of the game.
        std::string url{"https://github.com/ZASUO01/SpinoCore-Engine"};
    };
}