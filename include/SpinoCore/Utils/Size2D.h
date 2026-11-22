// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once

/// @brief Small helpers shared by the engine and games.
namespace SpinoCore::Utils {
    /// @brief Width and height in pixels.
    struct Size2D final {
        /// Width in pixels.
        int width;
        /// Height in pixels.
        int height;

        /// @brief Compares both dimensions.
        friend constexpr bool operator==(const Size2D&, const Size2D&) = default;
    };
}
