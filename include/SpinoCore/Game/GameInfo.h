// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <string>

namespace SpinoCore::Game {
    struct GameInfo final {
        std::string name{"SpinoCore App"};
        std::string version{"0.1.0"};
        std::string identifier{"io.github.zasuo01.spinocore"};
        std::string creator{"ZASUO01"};
        std::string url{"https://github.com/ZASUO01/SpinoCore-Engine"};
    };
}