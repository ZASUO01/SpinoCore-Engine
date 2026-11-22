// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SimulationInput.h"

namespace SpinoCore::Core::Input {
    void SimulationInput::BeginFrame(const InputState& frame) {
        if (mWasConsumed) {
            mState.BeginFrame();
            mWasConsumed = false;
        }
        mState.Accumulate(frame);
    }

    const InputState& SimulationInput::BeginTick() noexcept {
        if (mWasConsumed) mState.BeginFrame();
        mWasConsumed = true;
        return mState;
    }

    void SimulationInput::Discard() noexcept {
        mState.BeginFrame();
        mWasConsumed = false;
    }
}
