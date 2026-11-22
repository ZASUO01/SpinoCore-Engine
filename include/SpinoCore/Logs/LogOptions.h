// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include "LogLevel.h"
#include "LogSink.h"

namespace SpinoCore::Logs {
    /// @brief Logging options given to Launch.
    struct LogOptions final {
        /// Lowest level written.
        Level level{Level::Info};
        /// Destination of the messages; nullptr writes colored lines to the console.
        std::unique_ptr<LogSink> sink{nullptr};
    };

}
