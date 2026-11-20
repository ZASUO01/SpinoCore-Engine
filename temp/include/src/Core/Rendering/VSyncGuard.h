// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <chrono>
#include <cstdint>
#include <optional>

namespace SpinoCore::Core::Rendering {
    enum class VSyncAction : std::uint8_t {
        None,
        Disable,
        Enable,
    };

    class VSyncGuard final{
    public:
        using Clock = std::chrono::steady_clock;
        using Duration = std::chrono::nanoseconds;

        [[nodiscard]] VSyncAction OnFrame(Clock::time_point now, float refreshRate) noexcept;
        [[nodiscard]] VSyncAction OnWindowVisibility(bool visible, Clock::time_point now) noexcept;
        [[nodiscard]] VSyncAction OnPresentationChanged(Clock::time_point now) noexcept;
        void OnEnableFailed(Clock::time_point now) noexcept;
        [[nodiscard]] Duration MinimumFramePeriod() const noexcept;

    private:
        [[nodiscard]] VSyncAction Retry(Clock::time_point now) noexcept;
        void FallBack(Clock::time_point now) noexcept;
        void StartSample(Clock::time_point now) noexcept;
        [[nodiscard]] double ExpectedRefreshRate() const noexcept;

        bool mTrusted{true};
        bool mHidden{false};
        float mRefreshRate{0.0f};
        std::optional<Clock::time_point> mSampleStart{};
        std::uint32_t mFramesInSample{0};
        Clock::time_point mFallbackStart{};
    };
}
