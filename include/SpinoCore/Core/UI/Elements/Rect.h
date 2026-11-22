// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <algorithm>
#include "Element.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::Input {
    class InputState;
}

namespace SpinoCore::Core::Rendering {
    struct UIDrawList;
}

namespace SpinoCore::Core::UI::Elements {
    /// @brief Settings of a colored rectangle.
    struct RectInput : ElementInput {
        /// RGBA fill color.
        Math::Vector4f backgroundColor = Math::Vector4f{0.0f, 0.0f, 0.0f, 1.0f};
        /// Opacity from 0 to 1, multiplied into every color.
        float opacity = 1.0f;
    };

    /// @brief Filled rectangle with optional rounded corners and border.
    class Rect : public Element {
    public:
        /// @brief Creates a rectangle.
        /// @param input Initial settings.
        explicit Rect(const RectInput& input = {})
            : Element(input), mOpacity(std::clamp(input.opacity, 0.0f, 1.0f)), mBackgroundColor(input.backgroundColor) {}

        /// @brief Changes the fill color.
        /// @param color RGBA color.
        void SetBackgroundColor(const Math::Vector4f& color) { mBackgroundColor = color; }
        /// @return The fill color.
        [[nodiscard]] const Math::Vector4f& GetBackgroundColor() const { return mBackgroundColor; }

        /// @brief Changes the opacity.
        /// @param opacity Value clamped to 0..1.
        void SetOpacity(const float opacity) { mOpacity = std::clamp(opacity, 0.0f, 1.0f); }
        /// @return The opacity.
        [[nodiscard]] float GetOpacity() const { return mOpacity; }

        /// @brief Rounds the corners.
        /// @param radius Radius in pixels, clamped to 0..50.
        void SetBorderRadius(const float radius) { mBorderRadius = std::clamp(radius, 0.0f, MAX_BORDER_RADIUS); }
        /// @return The corner radius in pixels.
        [[nodiscard]] float GetBorderRadius() const { return mBorderRadius; }

        /// @brief Draws a border inside the edges.
        /// @param width Border width in pixels; negative values become 0.
        /// @param color RGBA border color.
        void SetBorder(const float width, const Math::Vector4f& color) { mBorderWidth = std::max(width, 0.0f); mBorderColor = color; }
        /// @return The border width in pixels.
        [[nodiscard]] float GetBorderWidth() const { return mBorderWidth; }
        /// @return The border color.
        [[nodiscard]] const Math::Vector4f& GetBorderColor() const { return mBorderColor; }

    protected:
        void OnProcessInput(const Input::InputState&) override {}
        void SetDrawCommand(Rendering::UIDrawList &drawList, int drawLayer) override;

    private:
        static constexpr float MAX_BORDER_RADIUS = 50.0f;

        float mOpacity = 1.0f;
        float mBorderRadius = 0.0f;
        float mBorderWidth = 0.0f;
        Math::Vector4f mBorderColor = Math::Vector4f{0.0f, 0.0f, 0.0f, 0.0f};
        Math::Vector4f mBackgroundColor = Math::Vector4f{0.0f, 0.0f, 0.0f, 1.0f};
    };
}
