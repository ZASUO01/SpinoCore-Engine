// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <algorithm>
#include "../../../../include/SpinoCore/Core/UI/Elements/Element.h"

namespace SpinoCore::Core::UI::Elements {
    class Rect : public Element {
    public:
        explicit Rect(const ElementInput& input = {}) : Element(input) {}

        void SetBackgroundColor(const Math::Vector4f& color) { mBackgroundColor = color; }
        [[nodiscard]] const Math::Vector4f& GetBackgroundColor() const { return mBackgroundColor; }

        void SetOpacity(const float opacity) { mOpacity = std::clamp(opacity, 0.0f, 1.0f); }
        [[nodiscard]] float GetOpacity() const { return mOpacity; }

    protected:
        void SendDrawCommand(Rendering::UIDrawList& drawList) override {
            Math::Vector4f color = mBackgroundColor;
            color.w *= mOpacity;

            drawList.push_back({
                .drawOrder = GetDrawOrder(),
                .transform = GetTransform(),
                .textureRect = Math::Vector4f{0.0f, 0.0f, 1.0f, 1.0f},
                .color = color,
                .size = Math::Vector2f{GetSize().x * GetWorldScale(), GetSize().y * GetWorldScale()},
                .borderRadius = 15.0f
            });
        }

    private:
        float mOpacity = 1.0f;
        Math::Vector4f mBackgroundColor = Math::Vector4f{0.0f, 0.0f, 0.0f, 1.0f};
    };
}
