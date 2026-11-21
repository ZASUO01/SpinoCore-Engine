// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/UI/Elements/Text.h"
#include <cmath>
#include <cstddef>
#include <memory>
#include <string_view>
#include <unordered_map>
#include "Assets/AssetCache.h"
#include "Core/Rendering/Drawer/DrawCommands.h"
#include "Graphics/Resources/Font.h"
#include "Graphics/Resources/Texture.h"
#include "SpinoCore/Logs/Logger.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/MatrixFactory.h"

namespace SpinoCore::Core::UI::Elements {
    namespace {
        constexpr char32_t REPLACEMENT_CHARACTER = U'?';

        [[nodiscard]] char32_t DecodeUTF8(const std::string_view text, std::size_t& index) {
            const auto lead = static_cast<unsigned char>(text[index++]);
            if (lead < 0x80) return lead;

            int continuationBytes = 0;
            char32_t codepoint = 0;
            if ((lead & 0xE0) == 0xC0) { continuationBytes = 1; codepoint = lead & 0x1F; }
            else if ((lead & 0xF0) == 0xE0) { continuationBytes = 2; codepoint = lead & 0x0F; }
            else if ((lead & 0xF8) == 0xF0) { continuationBytes = 3; codepoint = lead & 0x07; }
            else return REPLACEMENT_CHARACTER;

            for (int i = 0; i < continuationBytes; ++i) {
                if (index >= text.size()) return REPLACEMENT_CHARACTER;
                const auto next = static_cast<unsigned char>(text[index]);
                if ((next & 0xC0) != 0x80) return REPLACEMENT_CHARACTER;
                codepoint = (codepoint << 6) | (next & 0x3F); // NOLINT
                ++index;
            }
            return codepoint;
        }

        [[nodiscard]] std::vector<std::u32string> SplitLines(const std::string_view text) {
            std::vector<std::u32string> lines(1);
            for (std::size_t index = 0; index < text.size();) {
                if (const char32_t codepoint = DecodeUTF8(text, index); codepoint == U'\n') {
                    lines.emplace_back();
                } else if (codepoint != U'\r') {
                    lines.back().push_back(codepoint);
                }
            }
            return lines;
        }

        [[nodiscard]] const Graphics::Resources::GlyphInfo* FindGlyph(const Graphics::Resources::FontAtlas& atlas, const char32_t codepoint) {
            if (const auto it = atlas.glyphs.find(codepoint); it != atlas.glyphs.end()) return &it->second;
            if (const auto it = atlas.glyphs.find(REPLACEMENT_CHARACTER); it != atlas.glyphs.end()) return &it->second;
            return nullptr;
        }

        [[nodiscard]] bool SameSize(const Math::Vector2f& a, const Math::Vector2f& b) {
            return a.x == b.x && a.y == b.y;
        }

        [[nodiscard]] float LineWidth(const Graphics::Resources::FontAtlas& atlas, const std::u32string& line) {
            float width = 0.0f;
            for (const char32_t codepoint : line) {
                if (const auto* glyph = FindGlyph(atlas, codepoint)) width += static_cast<float>(glyph->advance);
            }
            return width;
        }
    }

    Text::Text(const TextInput &input)
    : Rect(input)
    , mText(input.text)
    , mFontName(input.font)
    , mFontSize(input.fontSize)
    , mColor(input.color)
    , mAlignment(input.alignment) {
        RectInput backgroundInput{};
        backgroundInput.size = input.size;
        backgroundInput.backgroundColor = input.backgroundColor;
        backgroundInput.drawOrder = input.drawOrder;
        mBackground = dynamic_cast<Rect*>(&AddChild(std::make_unique<Rect>(backgroundInput)));
    }

    void Text::ResolveFont(const Rendering::UIDrawList &drawList) {
        const auto* fonts = drawList.fonts;
        const std::uint64_t cacheVersion = fonts ? fonts->GetVersion() : 0;
        if (mIsFontCached && mCachedFrom == fonts && mCachedVersion == cacheVersion) return;

        const Graphics::Resources::Font* font = nullptr;
        if (fonts) {
            if (!mFontName.empty() && fonts->Contains(mFontName)) {
                font = &fonts->Get(mFontName);
            } else if (fonts->HasDefault()) {
                font = &fonts->GetDefault();
                if (!mFontName.empty() && !mWarnedMissingFont) {
                    Logs::Logger::Warn(R"([UI] Font "{}" is not cached (yet); the default font is drawn instead.)", mFontName);
                    mWarnedMissingFont = true;
                }
            }
        }

        const Graphics::Resources::FontAtlas* atlas = font ? font->GetAtlas(mFontSize) : nullptr;
        if (atlas != mCachedAtlas) mIsLayoutDirty = true;

        mCachedAtlas = atlas;
        mCachedFrom = fonts;
        mCachedVersion = cacheVersion;
        mIsFontCached = true;
    }

    void Text::Layout() {
        mGlyphs.clear();
        mLayoutSize = GetSize();
        mIsLayoutDirty = false;
        if (!mCachedAtlas) return;

        const auto& atlas = *mCachedAtlas;
        const auto lines = SplitLines(mText);
        const auto lineHeight = static_cast<float>(atlas.lineHeight);
        const float blockHeight = lineHeight * static_cast<float>(lines.size());

        float lineTop = std::round(-blockHeight * 0.5f);
        for (const auto& line : lines) {
            const float width = LineWidth(atlas, line);
            float penX = -mLayoutSize.x * 0.5f;
            if (mAlignment == TextAlignment::Center) penX = -width * 0.5f;
            else if (mAlignment == TextAlignment::Right) penX = mLayoutSize.x * 0.5f - width;
            penX = std::round(penX);

            for (const char32_t codepoint : line) {
                const auto* glyph = FindGlyph(atlas, codepoint);
                if (!glyph) continue;

                if (glyph->width > 0 && glyph->height > 0) {
                    mGlyphs.push_back(PlacedGlyph{
                        .topLeft = {penX + static_cast<float>(glyph->bearingX), lineTop + static_cast<float>(glyph->bearingY)},
                        .size = {static_cast<float>(glyph->width), static_cast<float>(glyph->height)},
                        .textureRect = {glyph->uMin, glyph->vMin, glyph->uMax - glyph->uMin, glyph->vMax - glyph->vMin},
                    });
                }
                penX += static_cast<float>(glyph->advance);
            }
            lineTop += lineHeight;
        }
    }

    void Text::SetDrawCommand(Rendering::UIDrawList &drawList, const int drawLayer) {
        if (!SameSize(mBackground->GetSize(), GetSize())) mBackground->SetSize(GetSize());
        if (mBackground->GetDrawOrder() != GetDrawOrder()) mBackground->SetDrawOrder(GetDrawOrder());

        ResolveFont(drawList);
        if (mIsLayoutDirty || !SameSize(mLayoutSize, GetSize())) Layout();
        if (!mCachedAtlas || mGlyphs.empty()) return;

        using namespace Math::MatrixFactory;

        const std::uint32_t textureId = mCachedAtlas->texture->GetID();
        const std::uint64_t key = Rendering::MakeUIDrawKey(drawLayer, GetDrawOrder(), textureId);
        const Math::Matrix4f& worldFrame = GetWorldFrame();
        const float worldScale = GetWorldScale();

        for (const auto& [topLeft, size, textureRect] : mGlyphs) {
            drawList.commands.push_back(Rendering::UIDrawCommand{
                .key = key,
                .transform = worldFrame * CreateTranslation(Math::Vector3f{topLeft.x, topLeft.y, 0.0f}) * CreateScale(size.x, size.y, 1.0f),
                .textureRect = textureRect,
                .color = mColor,
                .size = {size.x * worldScale, size.y * worldScale},
                .textureId = textureId,
                .textureMix = 1.0f,
                .opacity = GetOpacity(),
                .tintTexture = true,
            });
        }
    }
}
