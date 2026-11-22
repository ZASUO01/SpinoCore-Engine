// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstddef>
#include <string>
#include "Rect.h"
#include "SpinoCore/Graphics/Resources/FontSize.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::Input {
    class InputState;
}

namespace SpinoCore::Core::Rendering {
    struct UIDrawList;
}

namespace SpinoCore::Core::UI::Elements {
    class Text;

    /// @brief Settings of an input field.
    struct InputFieldInput final : RectInput {
        /// Initial UTF-8 text.
        std::string text{};
        /// Name of a font in the font cache; empty uses the default font.
        std::string font{};
        /// Font size.
        Graphics::Resources::FontSize fontSize{Graphics::Resources::FontSize::Size16};
        /// RGBA text color.
        Math::Vector4f textColor{0.0f, 0.0f, 0.0f, 1.0f};
        /// Space between the edges and the text, in pixels.
        float padding{8.0f};
    };

    /// @brief Single line text box; a click focuses it and typed text is appended.
    class InputField final : public Rect {
    public:
        /// Maximum number of characters.
        static constexpr std::size_t MAX_LENGTH = 100;

        /// @brief Creates an input field.
        /// @param input Initial settings.
        explicit InputField(const InputFieldInput& input);

        /// @brief Replaces the text.
        /// @param text UTF-8 text.
        void SetText(const std::string& text) const;
        /// @return The UTF-8 text.
        [[nodiscard]] const std::string& GetText() const;

        /// @brief Gives or removes the keyboard focus.
        /// @param focused true to receive typed text.
        void SetFocused(const bool focused) { mIsFocused = focused; }
        /// @return Whether the field receives typed text.
        [[nodiscard]] bool IsFocused() const override { return mIsFocused; }

        /// @return The text element inside the field.
        [[nodiscard]] Text& GetTextElement() const { return *mText; }

    protected:
        void OnProcessInput(const Input::InputState& state) override;
        void SetDrawCommand(Rendering::UIDrawList& drawList, int drawLayer) override;

    private:
        [[nodiscard]] Math::Vector2f TextSize() const;

        Text* mText{nullptr};
        float mPadding;
        bool mIsFocused{false};
    };
}
