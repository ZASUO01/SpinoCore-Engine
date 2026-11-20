// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include "LogSink.h"
#include "SpinoCore/Logs/LogLevel.h"

namespace SpinoCore::Logs {
    struct LogOptions final {
        Level level{Level::Info};
        std::unique_ptr<LogSink> sink{nullptr};
    };

}
