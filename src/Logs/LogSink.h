// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <chrono>
#include "SpinoCore/Logs/Logger.h"

namespace SpinoCore::Logs {
    using namespace std::chrono;

    class LogSink {
    public:
        virtual ~LogSink() = default;
        virtual void Write(Level  level, const zoned_time<system_clock::duration>& time, std::string_view message) = 0;
    };
}