// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "SpinoCore/Core/Input/InputState.h"

namespace SpinoCore::Core::Input {
    class SimulationInput final {
    public:
        void BeginFrame(const InputState& frame);
        const InputState& BeginTick() noexcept;
        void Discard() noexcept;

        [[nodiscard]] const InputState& GetState() const noexcept { return mState; }

    private:
        InputState mState{};
        bool mWasConsumed{false};
    };
}
