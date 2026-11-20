// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <chrono>
#include <gtest/gtest.h>
#include "Core/Rendering/Presenter/VSyncGuard.h"

namespace {
    using namespace std::chrono_literals;
    using SpinoCore::Core::Rendering::VSyncAction;
    using SpinoCore::Core::Rendering::VSyncGuard;

    using Clock = VSyncGuard::Clock;
    using Duration = VSyncGuard::Duration;

    constexpr Clock::time_point Start{};
    constexpr float DisplayRefreshRate = 60.0f;

    [[nodiscard]] Duration PeriodOf(const double framesPerSecond) {
        return std::chrono::duration_cast<Duration>(std::chrono::duration<double>(1.0 / framesPerSecond));
    }

    [[nodiscard]] bool DisablesWhileRunningAt(VSyncGuard& guard, const double framesPerSecond, const Duration runTime) {
        const Duration framePeriod = PeriodOf(framesPerSecond);
        for (Duration elapsed{0}; elapsed <= runTime; elapsed += framePeriod) {
            if (guard.OnFrame(Start + elapsed, DisplayRefreshRate) == VSyncAction::Disable) return true;
        }
        return false;
    }

    TEST(VSyncGuardTests, LimitsFramesToOneAndAHalfTimesTheDisplayRate) {
        VSyncGuard guard;

        (void)guard.OnFrame(Start, DisplayRefreshRate);

        EXPECT_EQ(guard.MinimumFramePeriod(), PeriodOf(DisplayRefreshRate * 1.5));
    }

    TEST(VSyncGuardTests, AssumesA240HzDisplayWhenTheRefreshRateIsUnknown) {
        VSyncGuard guard;

        (void)guard.OnFrame(Start, 0.0f);

        EXPECT_EQ(guard.MinimumFramePeriod(), PeriodOf(240.0 * 1.5));
    }

    TEST(VSyncGuardTests, KeepsVSyncWhenFramesFollowTheDisplayRate) {
        VSyncGuard guard;

        EXPECT_FALSE(DisablesWhileRunningAt(guard, DisplayRefreshRate, 2s));
    }

    TEST(VSyncGuardTests, DisablesVSyncWhenFramesRunMuchFasterThanTheDisplay) {
        VSyncGuard guard;

        EXPECT_TRUE(DisablesWhileRunningAt(guard, 500.0, 1s));
    }

    TEST(VSyncGuardTests, LimitsFramesToSixtyPerSecondAfterVSyncFails) {
        VSyncGuard guard;

        guard.OnEnableFailed(Start);

        EXPECT_EQ(guard.MinimumFramePeriod(), PeriodOf(60.0));
    }

    TEST(VSyncGuardTests, WaitsFiveSecondsBeforeRetryingVSync) {
        VSyncGuard guard;
        guard.OnEnableFailed(Start);

        const VSyncAction beforeRetry = guard.OnFrame(Start + 4s, DisplayRefreshRate);
        const VSyncAction atRetry = guard.OnFrame(Start + 5s, DisplayRefreshRate);

        EXPECT_EQ(beforeRetry, VSyncAction::None);
        EXPECT_EQ(atRetry, VSyncAction::Enable);
    }

    TEST(VSyncGuardTests, DisablesVSyncWhenTheWindowIsHidden) {
        VSyncGuard guard;

        EXPECT_EQ(guard.OnWindowVisibility(false, Start), VSyncAction::Disable);
    }

    TEST(VSyncGuardTests, LimitsFramesToSixtyPerSecondWhileHidden) {
        VSyncGuard guard;

        (void)guard.OnWindowVisibility(false, Start);

        EXPECT_EQ(guard.MinimumFramePeriod(), PeriodOf(60.0));
    }

    TEST(VSyncGuardTests, EnablesVSyncWhenTheWindowIsShownAgain) {
        VSyncGuard guard;
        (void)guard.OnWindowVisibility(false, Start);

        EXPECT_EQ(guard.OnWindowVisibility(true, Start + 1s), VSyncAction::Enable);
    }

    TEST(VSyncGuardTests, IgnoresAVisibilityEventThatChangesNothing) {
        VSyncGuard guard;

        EXPECT_EQ(guard.OnWindowVisibility(true, Start), VSyncAction::None);
    }

    TEST(VSyncGuardTests, RetriesVSyncWhenThePresentationChanges) {
        VSyncGuard guard;
        guard.OnEnableFailed(Start);

        EXPECT_EQ(guard.OnPresentationChanged(Start + 1s), VSyncAction::Enable);
    }

    TEST(VSyncGuardTests, IgnoresPresentationChangesWhileHidden) {
        VSyncGuard guard;
        (void)guard.OnWindowVisibility(false, Start);

        EXPECT_EQ(guard.OnPresentationChanged(Start + 1s), VSyncAction::None);
    }
}
