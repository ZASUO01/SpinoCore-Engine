// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <algorithm>
#include <atomic>
#include <chrono>
#include <limits>
#include "Defs/ClockConcept.h"
#include "Defs/FrameOperationsConcept.h"

namespace SpinoCore::Runtime {
    namespace chr = std::chrono;

    enum class FrameRateTarget : std::uint32_t {
        Unlimited = 0,
        Debug = 1,
        Console = 30,
        Standard = 60,
        High = 120,
        Esports = 144,
        Extreme = 240
    };

    struct LoopSettings {
        std::uint32_t simulationRateHz{60};
        std::uint32_t targetFrameRate{60};
        std::uint32_t maxStepsPerFrame{8};
    };

    template <Defs::IsClock ClockT>
    class LoopDriver final {
    public:
        explicit LoopDriver(const LoopSettings& settings = {}) noexcept
            : mFixedDeltaTime{PeriodOf(std::max(settings.simulationRateHz, 1u))},
              mMaxStepsPerFrame{std::max(settings.maxStepsPerFrame, 1u)},
              mTargetFrameTime{PeriodOf(ClampFrameRate(settings.targetFrameRate))} {}

        LoopDriver(const LoopDriver&) = delete;
        LoopDriver& operator=(const LoopDriver&) = delete;

        template <Defs::IsFrameOperator FrameOperatorT>
        void Run(FrameOperatorT& operations) {
            const float deltaTime = chr::duration<float>(mFixedDeltaTime).count();
            chr::nanoseconds accumulator{0};
            auto previousTime = ClockT::now();
            mNextDeadline = previousTime;

            mIsRunning.store(true, std::memory_order_relaxed);
            while (mIsRunning.load(std::memory_order_relaxed)) {
                ApplyPendingFrameRate();

                const auto frameStart = ClockT::now();
                const auto frameTime = frameStart - previousTime;
                previousTime = frameStart;

                if (!operations.ProcessEvents()) {
                    break;
                }

                if (!mIsPaused.load(std::memory_order_relaxed)) {
                    accumulator += frameTime;
                }

                std::uint32_t steps = 0;
                while (accumulator >= mFixedDeltaTime && steps < mMaxStepsPerFrame) {
                    operations.UpdateSimulation(deltaTime);
                    accumulator -= mFixedDeltaTime;
                    ++steps;
                }

                if (accumulator >= mFixedDeltaTime) {
                    accumulator %= mFixedDeltaTime;
                }

                operations.InterpolateEntities(chr::duration<float>(accumulator) / chr::duration<float>(mFixedDeltaTime));
                operations.Render();

                UpdateFPSCounter(frameTime);
                WaitForNextFrame();
            }
            mIsRunning.store(false, std::memory_order_relaxed);
        }

        void SetTargetFrameRate(const std::uint32_t framesPerSecond) noexcept {
            mRequestedFrameRate.store(ClampFrameRate(framesPerSecond), std::memory_order_relaxed);
        }
        void SetTargetFrameRate(const FrameRateTarget target) noexcept {
            SetTargetFrameRate(static_cast<std::uint32_t>(target));
        }
        void SetPaused(const bool paused) noexcept { mIsPaused.store(paused, std::memory_order_relaxed); }
        void Stop() noexcept { mIsRunning.store(false, std::memory_order_relaxed); }

        [[nodiscard]] bool IsRunning() const noexcept { return mIsRunning.load(std::memory_order_relaxed); }
        [[nodiscard]] bool IsPaused() const noexcept { return mIsPaused.load(std::memory_order_relaxed); }
        [[nodiscard]] float GetCurrentFPS() const noexcept { return mCurrentFPS.load(std::memory_order_relaxed); }

    private:
        static constexpr std::uint32_t kNoRequest = std::numeric_limits<std::uint32_t>::max();
        static constexpr std::uint32_t kMaxFrameRate = 1000;
        static constexpr chr::nanoseconds kSampleInterval{chr::milliseconds{500}};

        [[nodiscard]] static constexpr std::uint32_t ClampFrameRate(const std::uint32_t framesPerSecond) noexcept {
            return std::min(framesPerSecond, kMaxFrameRate);
        }

        [[nodiscard]] static constexpr chr::nanoseconds PeriodOf(const std::uint32_t hertz) noexcept {
            return hertz == 0 ? chr::nanoseconds{0} : chr::nanoseconds{chr::seconds{1}} / hertz;
        }

        void ApplyPendingFrameRate() noexcept {
            if (const auto requested = mRequestedFrameRate.exchange(kNoRequest, std::memory_order_relaxed);
                requested != kNoRequest) {
                mTargetFrameTime = PeriodOf(requested);
                mNextDeadline = ClockT::now();
            }
        }

        void WaitForNextFrame() noexcept {
            if (mTargetFrameTime == chr::nanoseconds{0}) {
                return;
            }

            mNextDeadline += mTargetFrameTime;
            const auto now = ClockT::now();

            if (now >= mNextDeadline) {
                if (now - mNextDeadline > mTargetFrameTime) {
                    mNextDeadline = now;
                }
                return;
            }

            ClockT::SleepUntil(mNextDeadline);
        }

        void UpdateFPSCounter(const chr::nanoseconds frameTime) noexcept {
            mSampleTime += frameTime;
            ++mSampleFrames;

            if (mSampleTime >= kSampleInterval) {
                mCurrentFPS.store(static_cast<float>(mSampleFrames / chr::duration<double>(mSampleTime).count()),
                                  std::memory_order_relaxed);
                mSampleTime = chr::nanoseconds{0};
                mSampleFrames = 0;
            }
        }

        const chr::nanoseconds mFixedDeltaTime;
        const std::uint32_t mMaxStepsPerFrame;

        chr::nanoseconds mTargetFrameTime;
        ClockT::time_point mNextDeadline{};

        std::atomic<bool> mIsRunning{false};
        std::atomic<bool> mIsPaused{false};
        std::atomic<std::uint32_t> mRequestedFrameRate{kNoRequest};

        std::atomic<float> mCurrentFPS{0.0f};
        chr::nanoseconds mSampleTime{0};
        std::uint32_t mSampleFrames{0};
    };
}