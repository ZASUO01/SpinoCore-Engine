// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Launch.h"

namespace SpinoCore {
    int Launch(
        std::unique_ptr<Game::GameApplication> /**game**/,
        const LaunchOptions& /**launchOptions**/
    ) {
        return EXIT_SUCCESS;
    }
}