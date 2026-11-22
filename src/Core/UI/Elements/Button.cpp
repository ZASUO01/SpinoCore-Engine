// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/UI/Elements/Button.h"
#include "SpinoCore/Core/Input/InputState.h"

namespace SpinoCore::Core::UI::Elements {
    void Button::OnProcessInput(const Input::InputState &state) {
        const bool hovered = state.IsMouseInWindow() && Contains(state.GetMousePosition());
        const bool entered = hovered && !mIsHovered;
        mIsHovered = hovered;

        if (entered && mOnHover) mOnHover();
        if (hovered && state.WasMouseButtonPressed(Input::MouseButton::Left)) {
            if (mOnClick) mOnClick();
            Select();
        }
    }
}
