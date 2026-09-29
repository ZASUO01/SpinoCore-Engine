// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <atomic>
#include <format>

namespace SpinoCore::Logs {
    enum class Level : uint8_t {
        Force = 0,
        Info = 1,
        Warn = 2,
        Error = 3,
        Fatal = 4,
        None = 5,
    };

    class Logger final {
    public:
        Logger() = delete;

        static void SetLevel(const Level level) {
            globalLevel.store(level, std::memory_order_relaxed);
        }

        template <typename... Args>
        static void Force(std::format_string<Args...> fmt, Args&&... args) {
            Submit(Level::Force, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void Info(std::format_string<Args...> fmt, Args&&... args) {
            if (globalLevel.load(std::memory_order_relaxed) > Level::Info) return;
            Submit(Level::Info, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void Warn(std::format_string<Args...> fmt, Args&&... args) {
            if (globalLevel.load(std::memory_order_relaxed) > Level::Warn) return;
            Submit(Level::Warn, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void Error(std::format_string<Args...> fmt, Args&&... args) {
            if (globalLevel.load(std::memory_order_relaxed) > Level::Error) return;
            Submit(Level::Error, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void Fatal(std::format_string<Args...> fmt, Args&&... args) {
            if (globalLevel.load(std::memory_order_relaxed) > Level::Fatal) return;
            Submit(Level::Fatal, std::format(fmt, std::forward<Args>(args)...));
        }

    private:
        friend class LogSystem;

        static void Submit(Level level, std::string&& message);

        static std::atomic<Level> globalLevel;
    };

}