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
    /// @brief Horizontal alignment of text inside its element.
    enum class TextAlignment : std::uint8_t {
        Left,
        Center,
        Right,
    };

    /// @brief What happens when text is wider than its element.
    enum class TextOverflow : std::uint8_t {
        /// The text is drawn past the edges.
        Visible,
        /// Glyphs past the edges are not drawn.
        Hidden,
        /// The end of the text stays visible, as in an input field.
        ScrollToEnd,
    };

    /// @brief Settings of a text element.
    struct TextInput final : RectInput {
        /// UTF-8 text.
        std::string text{};
        /// Name of a font in the font cache; empty uses the default font.
        std::string font{};
        /// Font size.
        Graphics::Resources::FontSize fontSize{Graphics::Resources::FontSize::Size16};
        /// RGBA text color.
        Math::Vector4f color{0.0f, 0.0f, 0.0f, 1.0f};
        /// Horizontal alignment.
        TextAlignment alignment{TextAlignment::Center};
        /// Overflow behaviour.
        TextOverflow overflow{TextOverflow::Visible};
    };

    /// @brief Single line of text over a background rectangle.
    class Text : public Rect {
    public:
        /// @brief Creates a text element; the background uses the rectangle settings of the input.
        /// @param input Initial settings.
        explicit Text(const TextInput& input);

        /// @brief Replaces the text.
        /// @param text UTF-8 text.
        void SetText(std::string text) { mText = std::move(text); mIsLayoutDirty = true; }
        /// @return The UTF-8 text.
        [[nodiscard]] const std::string& GetText() const { return mText; }

        /// @brief Changes the font.
        /// @param font Name in the font cache; a missing font draws the default one.
        void SetFont(std::string font) { mFontName = std::move(font); InvalidateFont(); }
        /// @return The font name.
        [[nodiscard]] const std::string& GetFont() const { return mFontName; }

        /// @brief Changes the font size.
        /// @param size New size.
        void SetFontSize(const Graphics::Resources::FontSize size) { mFontSize = size; InvalidateFont(); }
        /// @return The font size.
        [[nodiscard]] Graphics::Resources::FontSize GetFontSize() const { return mFontSize; }

        /// @brief Changes the text color.
        /// @param color RGBA color.
        void SetColor(const Math::Vector4f& color) { mColor = color; }
        /// @return The text color.
        [[nodiscard]] const Math::Vector4f& GetColor() const { return mColor; }

        /// @brief Changes the alignment.
        /// @param alignment New alignment.
        void SetAlignment(const TextAlignment alignment) { mAlignment = alignment; mIsLayoutDirty = true; }
        /// @return The alignment.
        [[nodiscard]] TextAlignment GetAlignment() const { return mAlignment; }

        /// @brief Changes the overflow behaviour.
        /// @param overflow New behaviour.
        void SetOverflow(const TextOverflow overflow) { mOverflow = overflow; mIsLayoutDirty = true; }
        /// @return The overflow behaviour.
        [[nodiscard]] TextOverflow GetOverflow() const { return mOverflow; }

        /// @return The background rectangle, a child of this element.
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
