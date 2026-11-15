// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <atomic>
#include <concepts>
#include <format>
#include <source_location>
#include <string>
#include <type_traits>
#include <utility>
#include "LogLevel.h"

namespace SpinoCore::Logs {
    template <typename... Args>
    struct BasicFormatWithLocation final {
        template <typename T>
        requires std::convertible_to<const T&, std::format_string<Args...>>
        consteval BasicFormatWithLocation(const T& fmt, // NOLINT(google-explicit-constructor)
                                          const std::source_location loc = std::source_location::current())
            : format(fmt), location(loc) {}

        std::format_string<Args...> format;
        std::source_location location;
    };

    template <typename... Args>
    using FormatWithLocation = BasicFormatWithLocation<std::type_identity_t<Args>...>;

    class Logger final {
    public:
        Logger() = delete;

        static void SetLevel(const Level level) noexcept {
            globalLevel.store(level, std::memory_order_relaxed);
        }

        template <typename... Args>
        static void Force(FormatWithLocation<Args...> fmt, Args&&... args) noexcept {
            Log(Level::Force, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void Info(FormatWithLocation<Args...> fmt, Args&&... args) noexcept {
            Log(Level::Info, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void Warn(FormatWithLocation<Args...> fmt, Args&&... args) noexcept {
            Log(Level::Warn, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void Error(FormatWithLocation<Args...> fmt, Args&&... args) noexcept {
            Log(Level::Error, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void Fatal(FormatWithLocation<Args...> fmt, Args&&... args) noexcept {
            Log(Level::Fatal, fmt, std::forward<Args>(args)...);
        }

    private:
        [[nodiscard]] static bool IsEnabled(const Level level) noexcept {
            return level == Level::Force || level >= globalLevel.load(std::memory_order_relaxed);
        }

        template <typename... Args>
        static void Log(const Level level, const FormatWithLocation<Args...>& fmt, Args&&... args) noexcept {
            if (!IsEnabled(level)) return;
            try {
                Submit(level, fmt.location, std::format(fmt.format, std::forward<Args>(args)...));
            } catch (...) {}
        }

        static void Submit(Level level, const std::source_location& location, std::string&& message) noexcept;

        static std::atomic<Level> globalLevel;
    };
}