// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include "Rect.h"
#include "SpinoCore/Graphics/Resources/FontSize.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::Rendering {
    struct UIDrawList;
}

namespace SpinoCore::Graphics::Resources {
    struct FontAtlas;
}

namespace SpinoCore::Core::UI::Elements {
    enum class TextAlignment : std::uint8_t {
        Left,
        Center,
        Right,
    };

    enum class TextOverflow : std::uint8_t {
        Visible,     // glyphs may be drawn outside the box
        Hidden,      // glyphs that leave the box are not drawn
        ScrollToEnd, // like Hidden, but a line wider than the box shows its end (as in a text field)
    };

    struct TextInput final : RectInput {
        std::string text{};
        std::string font{};
        Graphics::Resources::FontSize fontSize{Graphics::Resources::FontSize::Size16};
        Math::Vector4f color{0.0f, 0.0f, 0.0f, 1.0f};
        TextAlignment alignment{TextAlignment::Center};
        TextOverflow overflow{TextOverflow::Visible};
    };

    class Text : public Rect {
    public:
        explicit Text(const TextInput& input);

        void SetText(std::string text) { mText = std::move(text); mIsLayoutDirty = true; }
        [[nodiscard]] const std::string& GetText() const { return mText; }

        void SetFont(std::string font) { mFontName = std::move(font); InvalidateFont(); }
        [[nodiscard]] const std::string& GetFont() const { return mFontName; }

        void SetFontSize(const Graphics::Resources::FontSize size) { mFontSize = size; InvalidateFont(); }
        [[nodiscard]] Graphics::Resources::FontSize GetFontSize() const { return mFontSize; }

        void SetColor(const Math::Vector4f& color) { mColor = color; }
        [[nodiscard]] const Math::Vector4f& GetColor() const { return mColor; }

        void SetAlignment(const TextAlignment alignment) { mAlignment = alignment; mIsLayoutDirty = true; }
        [[nodiscard]] TextAlignment GetAlignment() const { return mAlignment; }

        void SetOverflow(const TextOverflow overflow) { mOverflow = overflow; mIsLayoutDirty = true; }
        [[nodiscard]] TextOverflow GetOverflow() const { return mOverflow; }

        [[nodiscard]] Rect& GetBackground() const { return *mBackground; }

    protected:
        void SetDrawCommand(Rendering::UIDrawList& drawList, int drawLayer) override;

    private:
        struct PlacedGlyph final {
            Math::Vector2f topLeft;
            Math::Vector2f size;
            Math::Vector4f textureRect;
        };

        void InvalidateFont() { mIsFontCached = false; mWarnedMissingFont = false; }
        void ResolveFont(const Rendering::UIDrawList& drawList);
        void Layout();

        std::string mText;
        std::string mFontName;
        Graphics::Resources::FontSize mFontSize;
        Math::Vector4f mColor;
        TextAlignment mAlignment;
        TextOverflow mOverflow;
        Rect* mBackground{nullptr};

        const void* mCachedFrom{nullptr};
        std::uint64_t mCachedVersion{0};
        const Graphics::Resources::FontAtlas* mCachedAtlas{nullptr};
        bool mIsFontCached{false};
        bool mWarnedMissingFont{false};

        std::vector<PlacedGlyph> mGlyphs;
        Math::Vector2f mLayoutSize{0.0f, 0.0f};
        bool mIsLayoutDirty{true};
    };
}
