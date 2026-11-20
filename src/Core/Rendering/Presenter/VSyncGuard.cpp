// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "VSyncGuard.h"

namespace SpinoCore::Core::Rendering {
    namespace {
        using namespace std::chrono_literals;

        constexpr double UnknownRefreshRate = 240.0;
        constexpr double DisplayTolerance = 1.25;
        constexpr double TrustedCeilingMargin = 1.5;
        constexpr VSyncGuard::Duration SampleWindow = 500ms;
        constexpr VSyncGuard::Duration RetryInterval = 5s;
        constexpr VSyncGuard::Duration FixedRatePeriod{1'000'000'000 / 60};
    }

    VSyncAction VSyncGuard::OnFrame(const Clock::time_point now, const float refreshRate) noexcept {
        mRefreshRate = refreshRate;
        if (mHidden) return VSyncAction::None;

        if (!mTrusted) {
            if (now - mFallbackStart < RetryInterval) return VSyncAction::None;
            return Retry(now);
        }

        if (!mSampleStart) {
            StartSample(now);
            return VSyncAction::None;
        }

        ++mFramesInSample;
        const auto sampleTime = now - *mSampleStart;
        if (sampleTime < SampleWindow) return VSyncAction::None;

        const double measuredFps = mFramesInSample / std::chrono::duration<double>(sampleTime).count();
        StartSample(now);
        if (measuredFps <= ExpectedRefreshRate() * DisplayTolerance) return VSyncAction::None;

        FallBack(now);
        return VSyncAction::Disable;
    }

    VSyncAction VSyncGuard::OnWindowVisibility(const bool visible, const Clock::time_point now) noexcept {
        if (visible == !mHidden) return VSyncAction::None;

        mHidden = !visible;
        if (visible) return Retry(now);

        mSampleStart.reset();
        return VSyncAction::Disable;
    }

    VSyncAction VSyncGuard::OnPresentationChanged(const Clock::time_point now) noexcept {
        return mHidden ? VSyncAction::None : Retry(now);
    }

    void VSyncGuard::OnEnableFailed(const Clock::time_point now) noexcept {
        FallBack(now);
    }

    VSyncGuard::Duration VSyncGuard::MinimumFramePeriod() const noexcept {
        if (mHidden || !mTrusted) return FixedRatePeriod;
        return std::chrono::duration_cast<Duration>(
            std::chrono::duration<double>(1.0 / (ExpectedRefreshRate() * TrustedCeilingMargin)));
    }

    VSyncAction VSyncGuard::Retry(const Clock::time_point now) noexcept {
        mTrusted = true;
        StartSample(now);
        return VSyncAction::Enable;
    }

    void VSyncGuard::FallBack(const Clock::time_point now) noexcept {
        mTrusted = false;
        mFallbackStart = now;
        mSampleStart.reset();
    }

    void VSyncGuard::StartSample(const Clock::time_point now) noexcept {
        mSampleStart = now;
        mFramesInSample = 0;
    }

    double VSyncGuard::ExpectedRefreshRate() const noexcept {
        return mRefreshRate > 0.0f ? mRefreshRate : UnknownRefreshRate;
    }
}