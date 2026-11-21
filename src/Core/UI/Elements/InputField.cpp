// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/UI/Elements/InputField.h"
#include <algorithm>
#include <memory>
#include <string_view>
#include <utility>
#include <SDL3/SDL_scancode.h>
#include "Core/Input/InputState.h"
#include "SpinoCore/Core/UI/Elements/Text.h"

namespace SpinoCore::Core::UI::Elements {
    namespace {
        [[nodiscard]] bool IsContinuationByte(const char byte) {
            return (static_cast<unsigned char>(byte) & 0xC0) == 0x80;
        }

        // Appends whole UTF-8 characters from `typed` while `text` has fewer than maxLength characters.
        void AppendLimited(std::string& text, const std::string_view typed, const std::size_t maxLength) {
            auto length = static_cast<std::size_t>(std::ranges::count_if(text, [](const char c) { return !IsContinuationByte(c); }));
            for (const char byte : typed) {
                if (!IsContinuationByte(byte) && length++ >= maxLength) break;
                text.push_back(byte);
            }
        }

        void RemoveLastCharacter(std::string& text) {
            while (!text.empty() && IsContinuationByte(text.back())) text.pop_back();
            if (!text.empty()) text.pop_back();
        }
    }

    InputField::InputField(const InputFieldInput &input)
    : Rect(input)
    , mPadding(std::max(input.padding, 0.0f)) {
        TextInput textInput{};
        textInput.size = TextSize();
        textInput.backgroundColor = {0.0f, 0.0f, 0.0f, 0.0f};
        textInput.drawOrder = input.drawOrder;
        textInput.font = input.font;
        textInput.fontSize = input.fontSize;
        textInput.color = input.textColor;
        textInput.alignment = TextAlignment::Left;
        textInput.overflow = TextOverflow::ScrollToEnd;
        auto text = std::make_unique<Text>(textInput);
        mText = text.get();
        AddChild(std::move(text));
        SetText(input.text);
    }

    void InputField::SetText(std::string text) {
        std::string limited;
        AppendLimited(limited, text, MAX_LENGTH);
        mText->SetText(std::move(limited));
    }

    const std::string& InputField::GetText() const {
        return mText->GetText();
    }

    void InputField::OnProcessInput(const Input::InputState &state) {
        if (state.WasMouseButtonPressed(Input::MouseButton::Left)) mIsFocused = Contains(state.GetMousePosition());
        if (!mIsFocused) return;

        std::string text = GetText();
        if (state.WasKeyTyped(SDL_SCANCODE_BACKSPACE)) RemoveLastCharacter(text);
        AppendLimited(text, state.GetTypedText(), MAX_LENGTH);
        if (text != GetText()) mText->SetText(std::move(text));
    }

    void InputField::SetDrawCommand(Rendering::UIDrawList &drawList, const int drawLayer) {
        Rect::SetDrawCommand(drawList, drawLayer);

        const Math::Vector2f textSize = TextSize();
        if (mText->GetSize().x != textSize.x || mText->GetSize().y != textSize.y) mText->SetSize(textSize);
        if (mText->GetDrawOrder() != GetDrawOrder()) mText->SetDrawOrder(GetDrawOrder());
    }

    Math::Vector2f InputField::TextSize() const {
        const Math::Vector2f& size = GetSize();
        return {std::max(size.x - 2.0f * mPadding, 0.0f), size.y};
    }
}
