// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "LogSystem.h"
#include "ConsoleSink.h"
#include "LogSink.h"

namespace SpinoCore::Logs {
    LogContext::LogContext()
    :mTimeZone(current_zone())
    {}

    void LogContext::SetSink(std::unique_ptr<LogSink> sink) {
        std::lock_guard lock(mMutex);
        mSink = std::move(sink);
    }

    void LogContext::Dispatch(const Level level, std::string &&message) {
        std::lock_guard lock(mMutex);
        if (!mSink) return;

        const auto now = system_clock::now();
        const zoned_time localTime{mTimeZone, now};
        mSink->Write(level, localTime, std::move(message));
    }

    // STATIC GLOBAL INSTANCE
    static std::unique_ptr<LogContext> globalLogContext{nullptr};

    std::atomic<Level> Logger::globalLevel{Level::Info};
    void Logger::Submit(const Level level, std::string &&message) {
        if (globalLogContext) {
            globalLogContext->Dispatch(level, std::move(message));
        }
    }

    void LogSystem::Initialize() {
        if (!globalLogContext) {
            globalLogContext = std::make_unique<LogContext>();
            globalLogContext->SetSink(std::make_unique<ConsoleSink>());
        }
    }

    void LogSystem::Shutdown() {
        if (!globalLogContext) return;

        globalLogContext.reset();
    }

    void LogSystem::SetSink(std::unique_ptr<LogSink> sink) {
        if (globalLogContext) {
            globalLogContext->SetSink(std::move(sink));
        }
    }
}