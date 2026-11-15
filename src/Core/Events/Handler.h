// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "EventBus.h"

namespace SpinoCore::Core::Events {
    class Handler final {
    public:
        Handler() = delete;

        [[nodiscard]] static bool PollEvents(EventBus& bus);
    };
}
