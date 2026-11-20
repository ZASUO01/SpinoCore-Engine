// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once

namespace SpinoCore::Runtime {
    class LoopingManager {
    public:
        virtual ~LoopingManager() = default;
        [[nodiscard]] virtual float GetMeasuredFPS() const noexcept = 0;
        virtual void Pause() noexcept = 0;
        virtual void Resume() noexcept = 0;
        virtual void Stop() noexcept = 0;
    };
}