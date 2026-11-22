// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>

/// @brief Logging.
namespace SpinoCore::Logs {
    /// @brief Severity of a log message.
    enum class Level : std::uint8_t {
        /// Always written, whatever the level.
        Force = 0,
        /// General information.
        Info = 1,
        /// Something unexpected that the engine recovered from.
        Warn = 2,
        /// An operation failed.
        Error = 3,
        /// The engine cannot continue.
        Fatal = 4,
        /// Used as a level to turn off everything except Force.
        None = 5,
    };
}