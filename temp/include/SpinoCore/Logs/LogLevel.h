// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>

namespace SpinoCore::Logs {
    enum class Level : std::uint8_t {
        Force = 0,
        Info = 1,
        Warn = 2,
        Error = 3,
        Fatal = 4,
        None = 5,
    };
}