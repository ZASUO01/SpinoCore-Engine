// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once

namespace SpinoCore::Utils {
    struct Size2D final {
        int width;
        int height;

        friend constexpr bool operator==(const Size2D&, const Size2D&) = default;
    };
}
