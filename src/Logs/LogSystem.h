// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <chrono>
#include <memory>
#include "SpinoCore/Logs/Logger.h"

namespace SpinoCore::Logs {
    class LogSink;

    class LogContext final {
    public:
        LogContext();
        void SetSink(std::unique_ptr<LogSink> sink);
        void Dispatch(Level, std::string&& message);

        LogContext(const LogContext&) = delete;
        LogContext& operator=(const LogContext&) = delete;
        LogContext(LogContext&&) = delete;
        LogContext& operator=(LogContext&&) = delete;

    private:
        std::mutex mMutex;
        std::unique_ptr<LogSink> mSink{nullptr};
        const std::chrono::time_zone* mTimeZone;
    };

    class LogSystem final {
    public:
        LogSystem() = delete;

        static void Initialize();
        static void Shutdown();
        static void SetSink(std::unique_ptr<LogSink> sink);
    };
}