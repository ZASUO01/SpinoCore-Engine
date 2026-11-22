// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/UI/Elements/Cursor.h"
#include "SpinoCore/Core/Input/InputState.h"

namespace SpinoCore::Core::UI::Elements {
    void Cursor::OnProcessInput(const Input::InputState &state) {
        SetIsVisible(state.IsMouseInWindow());
        SetOffset(state.GetMousePosition());
    }
}
