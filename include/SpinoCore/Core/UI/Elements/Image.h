// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <string>
#include <utility>
#include "Rect.h"

namespace SpinoCore::Core::UI::Elements {
    struct ImageInput final : RectInput {
        std::string name{"default"};
    };

    class Image final : public Rect {
    public:
        explicit Image(const ImageInput& input) : Rect(input), mTextureName(input.name) {}

        void SetTexture(std::string name) { mTextureName = std::move(name); mWarnedMissingTexture = false; }
        [[nodiscard]] const std::string& GetTexture() const { return mTextureName; }

    protected:
        void SetDrawCommand(Rendering::UIDrawList &drawList, int drawLayer) override;

    private:
        std::string mTextureName;
        bool mWarnedMissingTexture{false};
    };
}
