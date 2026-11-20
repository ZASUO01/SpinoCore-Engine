// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <gtest/gtest.h>
#include "Core/Window/WindowSizeHelpers.h"
#include "SpinoCore/Core/Window/WindowSize.h"

namespace {
    using namespace SpinoCore::Core::Window;

    TEST(WindowSizeHelpers, ListedSizeIsStandard) {
        EXPECT_TRUE(IsStandardWindowSize({.width = 1280, .height = 720}));
    }

    TEST(WindowSizeHelpers, UnlistedSizeIsNotStandard) {
        EXPECT_FALSE(IsStandardWindowSize({.width = 1000, .height = 700}));
    }

    TEST(WindowSizeHelpers, SizeEqualToTheAvailableAreaFits) {
        EXPECT_TRUE(Fits({.width = 1280, .height = 720}, {.width = 1280, .height = 720}));
    }

    TEST(WindowSizeHelpers, SizeWiderThanTheAvailableAreaDoesNotFit) {
        EXPECT_FALSE(Fits({.width = 1281, .height = 720}, {.width = 1280, .height = 720}));
    }

    TEST(WindowSizeHelpers, SizeTallerThanTheAvailableAreaDoesNotFit) {
        EXPECT_FALSE(Fits({.width = 1280, .height = 721}, {.width = 1280, .height = 720}));
    }

    TEST(WindowSizeHelpers, StandardSizeThatFitsIsKept) {
        constexpr Size2D fitted = FitStandardWindowSize(WINDOW_SIZE_MEDIUM, WINDOW_SIZE_MAX);

        EXPECT_EQ(fitted, WINDOW_SIZE_MEDIUM);
    }

    TEST(WindowSizeHelpers, NonStandardSizeBecomesTheLargestStandardSizeInsideIt) {
        constexpr Size2D fitted = FitStandardWindowSize({.width = 1700, .height = 1000}, WINDOW_SIZE_MAX);

        EXPECT_EQ(fitted, WINDOW_SIZE_EXTRA_LARGE);
    }

    TEST(WindowSizeHelpers, SizeLargerThanTheAvailableAreaShrinksToTheLargestThatFits) {
        constexpr Size2D fitted = FitStandardWindowSize(WINDOW_SIZE_MAX, {.width = 1400, .height = 800});

        EXPECT_EQ(fitted, WINDOW_SIZE_LARGE);
    }

    TEST(WindowSizeHelpers, FallsBackToTheMinimumSizeWhenNothingFits) {
        constexpr Size2D fitted = FitStandardWindowSize(WINDOW_SIZE_MAX, {.width = 640, .height = 480});

        EXPECT_EQ(fitted, WINDOW_SIZE_MIN);
    }

    TEST(WindowSizeHelpers, SizeSmallerThanTheMinimumGrowsToTheMinimum) {
        constexpr Size2D fitted = FitStandardWindowSize({.width = 640, .height = 480}, WINDOW_SIZE_MAX);

        EXPECT_EQ(fitted, WINDOW_SIZE_MIN);
    }
}
