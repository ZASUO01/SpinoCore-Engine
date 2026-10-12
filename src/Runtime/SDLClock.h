// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <chrono>
#include <SDL3/SDL_timer.h>

namespace SpinoCore::Runtime {
    struct SDLClock final {
        using duration = std::chrono::nanoseconds;
        using rep = duration::rep;
        using period = duration::period;
        using time_point = std::chrono::time_point<SDLClock>;
        static constexpr bool is_steady = true;

        [[nodiscard]] static time_point now() noexcept {
            return time_point{duration{static_cast<rep>(SDL_GetTicksNS())}};
        }

        static void SleepUntil(const time_point deadline) noexcept {
            if (const auto remaining = deadline - now(); remaining > duration::zero()) {
                SDL_DelayPrecise(static_cast<Uint64>(remaining.count()));
            }
        }
    };
}