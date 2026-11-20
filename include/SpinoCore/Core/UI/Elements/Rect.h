// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <algorithm>
#include "Element.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::UI::Elements {
    struct RectInput : ElementInput {
        Math::Vector4f backgroundColor = Math::Vector4f{0.0f, 0.0f, 0.0f, 1.0f};
        float opacity = 1.0f;
    };

    class Rect : public Element {
    public:
        explicit Rect(const RectInput& input = {})
            : Element(input), mOpacity(std::clamp(input.opacity, 0.0f, 1.0f)), mBackgroundColor(input.backgroundColor) {}

        void SetBackgroundColor(const Math::Vector4f& color) { mBackgroundColor = color; }
        [[nodiscard]] const Math::Vector4f& GetBackgroundColor() const { return mBackgroundColor; }

        void SetOpacity(const float opacity) { mOpacity = std::clamp(opacity, 0.0f, 1.0f); }
        [[nodiscard]] float GetOpacity() const { return mOpacity; }

        void SetBorderRadius(const float radius) { mBorderRadius = std::clamp(radius, 0.0f, MAX_BORDER_RADIUS); }
        [[nodiscard]] float GetBorderRadius() const { return mBorderRadius; }

    protected:
        void SetDrawCommand(Rendering::UIDrawList &drawList, int drawLayer) override;

    private:
        static constexpr float MAX_BORDER_RADIUS = 50.0f;
        
        float mOpacity = 1.0f;
        float mBorderRadius = 0.0f;
        Math::Vector4f mBackgroundColor = Math::Vector4f{0.0f, 0.0f, 0.0f, 1.0f};
    };
}
