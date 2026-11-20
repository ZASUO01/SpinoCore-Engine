// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>

namespace SpinoCore::Logs {
    class LogSink;
    struct LogRecord;

    class LogContext final {
    public:
        explicit LogContext(std::unique_ptr<LogSink> sink);
        ~LogContext();

        LogContext(const LogContext&) = delete;
        LogContext& operator=(const LogContext&) = delete;
        LogContext(LogContext&&) = delete;
        LogContext& operator=(LogContext&&) = delete;

        void Dispatch(const LogRecord& record) const;

    private:
        std::unique_ptr<LogSink> mSink;
    };

    class LogSystem final {
    public:
        explicit LogSystem(std::unique_ptr<LogSink> sink = nullptr);
        ~LogSystem();

        LogSystem(const LogSystem&) = delete;
        LogSystem& operator=(const LogSystem&) = delete;
        LogSystem(LogSystem&&) = delete;
        LogSystem& operator=(LogSystem&&) = delete;

    private:
        LogContext mContext;
    };
}