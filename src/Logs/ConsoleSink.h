// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <iostream>
#include "LogSink.h"

namespace SpinoCore::Logs {
    class ConsoleSink final : public LogSink {
    public:
        void Write(const Level level, const zoned_time<system_clock::duration> &time, std::string_view message) override {
            std::string_view color;
            std::string_view  prefix;

            switch (level) {
                case Level::Force: color = "\033[36m";    prefix = "[CORE]";   break;
                case Level::Info:  color = "\033[32m";    prefix = "[INFO]";   break;
                case Level::Warn:  color = "\033[33m";    prefix = "[WARN]";   break;
                case Level::Error: color = "\033[31m";    prefix = "[ERROR]";  break;
                case Level::Fatal: color = "\033[41;37m"; prefix = "[FATAL]";  break;
                default:           color = "\033[0m";     prefix = "[TRACE]";  break;
            }

            std::cerr  << std::format("{}{} {:%H:%M:%S} - {}{}\n", color, prefix, time, message, "\033[0m");
        }
    };
}