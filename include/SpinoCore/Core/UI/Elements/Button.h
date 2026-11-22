// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <functional>
#include <utility>
#include "Text.h"

namespace SpinoCore::Core::Input {
    class InputState;
}

namespace SpinoCore::Core::UI::Elements {
    class Button final : public Text {
    public:
        using Callback = std::function<void()>;

        explicit Button(const TextInput& input) : Text(input) {}

        void SetOnClick(Callback callback) { mOnClick = std::move(callback); }
        void SetOnHover(Callback callback) { mOnHover = std::move(callback); }
        void SetOnSelect(Callback callback) { mOnSelect = std::move(callback); }

        void Select() const { if (mOnSelect) mOnSelect(); }

        [[nodiscard]] bool IsHovered() const { return mIsHovered; }

    protected:
        void OnProcessInput(const Input::InputState& state) override;

    private:
        Callback mOnClick;
        Callback mOnHover;
        Callback mOnSelect;
        bool mIsHovered{false};
    };
}
