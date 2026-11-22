// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <chrono>
#include <source_location>
#include <string_view>
#include "LogLevel.h"

namespace SpinoCore::Logs {
    /// @brief One log message.
    struct LogRecord final {
        /// Severity.
        Level level;
        /// When the message was logged.
        std::chrono::system_clock::time_point time;
        /// Where the message was logged.
        std::source_location location;
        /// Formatted text, valid only during LogSink::Write.
        std::string_view message;
    };

    /// @brief Destination of log messages.
    class LogSink {
    public:
        virtual ~LogSink() = default;
        /// @brief Writes a message; calls are serialized by the engine.
        /// @param record Message to write.
        virtual void Write(const LogRecord& record) = 0;
    };
}