// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <chrono>
#include <mutex>
#include <utility>
#include <stdexcept>
#include <memory>
#include "LogSystem.h"
#include "ConsoleSink.h"
#include "SpinoCore/Logs/Logger.h"
#include "SpinoCore/Logs/LogSink.h"

namespace SpinoCore::Logs {
    namespace {
        constinit std::mutex gMutex;
        constinit LogContext* gContext = nullptr;
        constinit thread_local bool tInSubmit = false;

        class ReentrancyGuard final {
        public:
            ReentrancyGuard() noexcept { tInSubmit = true; }
            ~ReentrancyGuard() { tInSubmit = false; }

            ReentrancyGuard(const ReentrancyGuard&) = delete;
            ReentrancyGuard& operator=(const ReentrancyGuard&) = delete;
            ReentrancyGuard(ReentrancyGuard&&) = delete;
            ReentrancyGuard& operator=(ReentrancyGuard&&) = delete;
        };
    }

    LogContext::LogContext(std::unique_ptr<LogSink> sink) : mSink(std::move(sink)) {}

    LogContext::~LogContext() = default;

    void LogContext::Dispatch(const LogRecord& record) const {
        if (!mSink) return;
        mSink->Write(record);
    }

    std::atomic<Level> Logger::globalLevel{Level::Info};

    void Logger::Submit(const Level level, const std::source_location& location, std::string&& message) noexcept {
        if (tInSubmit) return;
        const ReentrancyGuard reentrancyGuard{};

        try {
            const LogRecord record{level, std::chrono::system_clock::now(), location, message};

            std::lock_guard lock(gMutex);
            if (gContext) {
                gContext->Dispatch(record);
            }
        } catch (...) {
        }
    }

    LogSystem::LogSystem(std::unique_ptr<LogSink> sink)
    :mContext(sink ? std::move(sink) : std::make_unique<ConsoleSink>())
    {
        std::lock_guard lock(gMutex);
        if (gContext) {
            throw std::logic_error("[LOG SYSTEM] Only one LogSystem may exist at a time");
        }
        gContext = &mContext;
    }

    LogSystem::~LogSystem() {
        std::lock_guard lock(gMutex);
        gContext = nullptr;
    }
}