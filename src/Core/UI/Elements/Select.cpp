// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/UI/Elements/Select.h"
#include <memory>
#include <stdexcept>
#include <utility>
#include "SpinoCore/Core/UI/Elements/Button.h"

namespace SpinoCore::Core::UI::Elements {
    namespace {
        [[nodiscard]] std::unique_ptr<Button> MakeArrow(const TextInput& selectInput, std::string arrow, const float offsetX) {
            TextInput input = selectInput;
            input.anchor = {0.0f, 0.0f};
            input.offSet = {offsetX, 0.0f};
            input.size = {selectInput.size.y, selectInput.size.y};
            input.text = std::move(arrow);
            input.alignment = TextAlignment::Center;
            return std::make_unique<Button>(input);
        }
    }

    Select::Select(const TextInput &input, std::vector<std::string> options)
    : Text(input)
    , mOptions(std::move(options)) {
        if (mOptions.empty()) throw std::invalid_argument("[UI] A select needs at least one option.");
        SetText(mOptions[0]);

        const float arrowOffset = (input.size.x + input.size.y) * 0.5f;

        auto previous = MakeArrow(input, "<", -arrowOffset);
        previous->SetOnSelect([this] { Previous(); });
        mPreviousButton = previous.get();
        AddChild(std::move(previous));

        auto next = MakeArrow(input, ">", arrowOffset);
        next->SetOnSelect([this] { Next(); });
        mNextButton = next.get();
        AddChild(std::move(next));
    }

    void Select::SetSelectedIndex(const std::size_t index) {
        if (index >= mOptions.size()) return;
        mSelectedIndex = index;
        SetText(mOptions[index]);
    }
}
