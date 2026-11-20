// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include "FrameOperations.h"
#include "SpinoCore/Runtime/LoopingManager.h"

namespace SpinoCore::Runtime {
    template <typename Clock = std::chrono::steady_clock>
    requires std::chrono::is_clock_v<Clock>
    class GameLooping final : public LoopingManager {
    public:
        using Duration = std::chrono::nanoseconds;

        GameLooping() = default;
        ~GameLooping() override = default;

        void Run(FrameOperations& operations) {
            constexpr float fixedDeltaSeconds = std::chrono::duration<float>(FixedDelta).count();
            Duration accumulator{0};
            auto previousTime = Clock::now();
            auto fpsSampleStart = previousTime;
            std::uint32_t framesInSample = 0;

            while (true) {
                const auto frameStart = Clock::now();
                const Duration frameTime = std::min<Duration>(
                  std::chrono::duration_cast<Duration>(frameStart - previousTime),
                  MaxFrameTime
                );
                previousTime = frameStart;

                if (!operations.ProcessEvents()) break;
                operations.ProcessFrameInputs();

                if (operations.PrepareSimulation()) {
                    if (!mPaused.load(std::memory_order_relaxed)) {
                        accumulator += frameTime;
                        while (accumulator >= FixedDelta) {
                            operations.ProcessSimulationInputs();
                            operations.UpdateSimulation(fixedDeltaSeconds);
                            accumulator -= FixedDelta;
                        }
                    }
                    operations.Interpolate(std::chrono::duration<float>(accumulator) / FixedDelta);
                }else {
                    accumulator = Duration{0};
                    previousTime = Clock::now();
                }
                operations.Render();

                if (mStopRequested.load(std::memory_order_relaxed)) break;

                const Duration minimumFramePeriod = operations.GetFramePeriod();
                if (const Duration workTime = std::chrono::duration_cast<Duration>(Clock::now() - frameStart); workTime < minimumFramePeriod) {
                    SDL_DelayPrecise(static_cast<Uint64>((minimumFramePeriod - workTime).count()));
                }

                ++framesInSample;
                const auto frameEnd = Clock::now();
                if (const auto sampleTime = frameEnd - fpsSampleStart; sampleTime >= FpsSampleInterval) {
                    mMeasuredFps.store(static_cast<float>(framesInSample) / std::chrono::duration<float>(sampleTime).count(),
                                       std::memory_order_relaxed);
                    framesInSample = 0;
                    fpsSampleStart = frameEnd;
                }
            }

            mStopRequested.store(false, std::memory_order_relaxed);
        }

        [[nodiscard]] float GetMeasuredFPS() const noexcept override { return mMeasuredFps.load(std::memory_order_relaxed); }
        void Pause() noexcept override { mPaused.store(true, std::memory_order_relaxed); }
        void Resume() noexcept override { mPaused.store(false, std::memory_order_relaxed); }
        void Stop() noexcept override { mStopRequested.store(true, std::memory_order_relaxed); }

    private:
        static constexpr Duration FixedDelta{1'000'000'000 / 60};
        static constexpr Duration MaxFrameTime{250'000'000};
        static constexpr Duration FpsSampleInterval{1'000'000'000};

        std::atomic<bool> mStopRequested{false};
        std::atomic<bool> mPaused{false};
        std::atomic<float> mMeasuredFps{0.0};
    };
}
