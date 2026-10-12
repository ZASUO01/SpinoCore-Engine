// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <chrono>
#include <concepts>

namespace SpinoCore::Defs {
    template <typename ClockT>
    concept IsClock = requires(typename ClockT::time_point deadline) {
        requires std::same_as<typename ClockT::duration, std::chrono::nanoseconds>;
        requires ClockT::is_steady;
        { ClockT::now() } noexcept -> std::same_as<typename ClockT::time_point>;
        { ClockT::SleepUntil(deadline) } noexcept -> std::same_as<void>;
    };
}