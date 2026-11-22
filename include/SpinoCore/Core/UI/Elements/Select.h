// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstddef>
#include <string>
#include <vector>
#include "Text.h"

namespace SpinoCore::Core::UI::Elements {
    class Button;

    class Select final : public Text {
    public:
        Select(const TextInput& input, std::vector<std::string> options);

        void Next() { SetSelectedIndex((mSelectedIndex + 1) % mOptions.size()); }
        void Previous() { SetSelectedIndex((mSelectedIndex + mOptions.size() - 1) % mOptions.size()); }

        void SetSelectedIndex(std::size_t index);
        [[nodiscard]] std::size_t GetSelectedIndex() const { return mSelectedIndex; }
        [[nodiscard]] const std::string& GetSelectedOption() const { return mOptions[mSelectedIndex]; }
        [[nodiscard]] const std::vector<std::string>& GetOptions() const { return mOptions; }

        [[nodiscard]] Button& GetPreviousButton() const { return *mPreviousButton; }
        [[nodiscard]] Button& GetNextButton() const { return *mNextButton; }

    private:
        std::vector<std::string> mOptions;
        std::size_t mSelectedIndex{0};
        Button* mPreviousButton{nullptr};
        Button* mNextButton{nullptr};
    };
}
