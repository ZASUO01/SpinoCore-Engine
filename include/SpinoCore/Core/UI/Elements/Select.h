// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstddef>
#include <string>
#include <vector>
#include "Text.h"

namespace SpinoCore::Core::UI::Elements {
    class Button;

    /// @brief Text element that cycles through options with previous and next buttons.
    class Select final : public Text {
    public:
        /// @brief Creates a select showing the first option.
        /// @param input Text and background settings.
        /// @param options Options to show.
        /// @throws std::invalid_argument When there are no options.
        Select(const TextInput& input, std::vector<std::string> options);

        /// @brief Shows the next option, wrapping to the first.
        void Next() { SetSelectedIndex((mSelectedIndex + 1) % mOptions.size()); }
        /// @brief Shows the previous option, wrapping to the last.
        void Previous() { SetSelectedIndex((mSelectedIndex + mOptions.size() - 1) % mOptions.size()); }

        /// @brief Shows an option.
        /// @param index Index of the option; an invalid index is ignored.
        void SetSelectedIndex(std::size_t index);
        /// @return The index of the shown option.
        [[nodiscard]] std::size_t GetSelectedIndex() const { return mSelectedIndex; }
        /// @return The shown option.
        [[nodiscard]] const std::string& GetSelectedOption() const { return mOptions[mSelectedIndex]; }
        /// @return Every option.
        [[nodiscard]] const std::vector<std::string>& GetOptions() const { return mOptions; }

        /// @return The button that shows the previous option.
        [[nodiscard]] Button& GetPreviousButton() const { return *mPreviousButton; }
        /// @return The button that shows the next option.
        [[nodiscard]] Button& GetNextButton() const { return *mNextButton; }

    private:
        std::vector<std::string> mOptions;
        std::size_t mSelectedIndex{0};
        Button* mPreviousButton{nullptr};
        Button* mNextButton{nullptr};
    };
}
