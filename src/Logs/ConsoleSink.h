// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <chrono>
#include <format>
#include <iostream>
#include <string>
#include <string_view>
#include "SpinoCore/Logs/LogLevel.h"
#include "SpinoCore/Logs/LogSink.h"

namespace SpinoCore::Logs {
    class ConsoleSink final : public LogSink {
    public:
        ConsoleSink() noexcept {
            try {
                mTimeZone = std::chrono::current_zone();
            } catch (...) {
                mTimeZone = nullptr;
            }
        }

        void Write(const LogRecord& record) override {
            std::string_view color;
            std::string_view prefix;

            switch (record.level) {
                case Level::Force: color = "\033[36m";    prefix = "[CORE]";   break;
                case Level::Info:  color = "\033[32m";    prefix = "[INFO]";   break;
                case Level::Warn:  color = "\033[33m";    prefix = "[WARN]";   break;
                case Level::Error: color = "\033[31m";    prefix = "[ERROR]";  break;
                case Level::Fatal: color = "\033[41;37m"; prefix = "[FATAL]";  break;
                default:           color = "\033[0m";     prefix = "[LOG]";    break;
            }

            const auto seconds = std::chrono::floor<std::chrono::seconds>(record.time);
            const std::string time = mTimeZone
                ? std::format("{:%H:%M:%S}", std::chrono::zoned_time{mTimeZone, seconds})
                : std::format("{:%H:%M:%S}Z", seconds);

            std::cerr << std::format("{}{} {} {}:{} - {}\033[0m\n",
                color, prefix, time, FileName(record.location.file_name()), record.location.line(), record.message);
        }

    private:
        [[nodiscard]] static std::string_view FileName(const std::string_view path) noexcept {
            const auto pos = path.find_last_of("/\\");
            return pos == std::string_view::npos ? path : path.substr(pos + 1);
        }

        const std::chrono::time_zone* mTimeZone{nullptr};
    };
}