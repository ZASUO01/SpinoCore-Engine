// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>

namespace SpinoCore {
    namespace App {
        class UserApplication;
    }

    using UniqueUserApp = std::unique_ptr<App::UserApplication>;

    [[nodiscard]] int Launch(UniqueUserApp app);
}
