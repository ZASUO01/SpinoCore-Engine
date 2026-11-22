// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <algorithm>
#include <array>
#include "SpinoCore/Core/Window/WindowSize.h"

namespace SpinoCore::Core::Window {
    inline constexpr std::array<Size2D, 6> STANDARD_WINDOW_SIZES {{
        WINDOW_SIZE_MAX,
        WINDOW_SIZE_EXTRA_LARGE,
        WINDOW_SIZE_LARGE,
        WINDOW_SIZE_MEDIUM,
        WINDOW_SIZE_SMALL,
        WINDOW_SIZE_MIN
    }};

    [[nodiscard]] constexpr bool IsStandardWindowSize(const Size2D size) noexcept {
        return std::ranges::find(STANDARD_WINDOW_SIZES, size) != STANDARD_WINDOW_SIZES.end();
    }

    [[nodiscard]] constexpr bool Fits(const Size2D size, const Size2D available) noexcept {
        return size.width <= available.width && size.height <= available.height;
    }

    [[nodiscard]] constexpr Size2D FitStandardWindowSize(const Size2D requested, const Size2D available) noexcept {
        const auto* const fitting = std::ranges::find_if(STANDARD_WINDOW_SIZES, [&](const Size2D size) {
            return Fits(size, requested) && Fits(size, available);
        });
        return fitting != STANDARD_WINDOW_SIZES.end() ? *fitting : WINDOW_SIZE_MIN;
    }
}
