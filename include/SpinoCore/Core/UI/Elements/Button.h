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
    /// @brief Text element that reacts to the mouse.
    class Button final : public Text {
    public:
        /// @brief Function called by a button event.
        using Callback = std::function<void()>;

        /// @brief Creates a button.
        /// @param input Text and background settings.
        explicit Button(const TextInput& input) : Text(input) {}

        /// @brief Sets the function called when the left button is pressed over the button.
        /// @param callback Function to call.
        void SetOnClick(Callback callback) { mOnClick = std::move(callback); }
        /// @brief Sets the function called when the mouse enters the button.
        /// @param callback Function to call.
        void SetOnHover(Callback callback) { mOnHover = std::move(callback); }
        /// @brief Sets the function called after a click or by Select.
        /// @param callback Function to call.
        void SetOnSelect(Callback callback) { mOnSelect = std::move(callback); }

        /// @brief Calls the select function, if any.
        void Select() const { if (mOnSelect) mOnSelect(); }

        /// @return Whether the mouse is over the button.
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
