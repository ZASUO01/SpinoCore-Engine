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

    struct InputFieldInput final : RectInput {
        std::string text{};
        std::string font{};
        Graphics::Resources::FontSize fontSize{Graphics::Resources::FontSize::Size16};
        Math::Vector4f textColor{0.0f, 0.0f, 0.0f, 1.0f};
        float padding{8.0f};
    };

    class InputField final : public Rect {
    public:
        static constexpr std::size_t MAX_LENGTH = 100;

        explicit InputField(const InputFieldInput& input);

        void SetText(const std::string& text) const;
        [[nodiscard]] const std::string& GetText() const;

        void SetFocused(const bool focused) { mIsFocused = focused; }
        [[nodiscard]] bool IsFocused() const override { return mIsFocused; }

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
