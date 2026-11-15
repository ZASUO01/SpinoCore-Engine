// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <chrono>
#include <source_location>
#include <string_view>
#include "LogLevel.h"

namespace SpinoCore::Logs {
    struct LogRecord final {
        Level level;
        std::chrono::system_clock::time_point time;
        std::source_location location;
        std::string_view message;
    };

    class LogSink {
    public:
        virtual ~LogSink() = default;
        virtual void Write(const LogRecord& record) = 0;
    };
}