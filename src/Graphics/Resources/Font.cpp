// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Font.h"
#include <algorithm>
#include <array>
#include <format>
#include <functional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>
#include <SDL3/SDL_blendmode.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "EmbeddedFonts.h"
#include "SpinoCore/Graphics/Resources/FontSize.h"
#include "SDL/Helpers/Helpers.h"
#include "SDL/Structs.h"
#include "Texture.h"

namespace SpinoCore::Graphics::Resources {
    namespace {
        constexpr int ATLAS_WIDTH = 512;
        constexpr int GLYPH_PADDING = 2;

        constexpr std::array FONT_SIZES{
            FontSize::Size10, FontSize::Size12, FontSize::Size16, FontSize::Size18,
            FontSize::Size24, FontSize::Size30, FontSize::Size36, FontSize::Size42,
            FontSize::Size48, FontSize::Size54, FontSize::Size60, FontSize::Size72,
        };

        constexpr std::array<std::pair<char32_t, char32_t>, 2> CHARACTER_RANGES{{{0x20, 0x7E}, {0xA0, 0xFF}}};

        struct RenderedGlyph final {
            char32_t codepoint;
            GlyphInfo info;
            SDL::UniqueSDLSurface image;
            SDL_Point position{0, 0};
        };

        FontAtlas GenerateAtlas(TTF_Font* font) {
            FontAtlas atlas;
            atlas.ascent = TTF_GetFontAscent(font);
            atlas.lineHeight = TTF_GetFontLineSkip(font);

            std::vector<RenderedGlyph> rendered;
            int penX = 0;
            int penY = 0;
            int rowHeight = 0;

            for (const auto& [first, last] : CHARACTER_RANGES) {
                for (char32_t codepoint = first; codepoint <= last; ++codepoint) {
                    if (!TTF_FontHasGlyph(font, codepoint)) continue;

                    int minX = 0, maxX = 0, minY = 0, maxY = 0, advance = 0;
                    if (!TTF_GetGlyphMetrics(font, codepoint, &minX, &maxX, &minY, &maxY, &advance)) continue;

                    RenderedGlyph glyph{codepoint, {}, nullptr};
                    glyph.info.advance = advance;
                    glyph.info.bearingX = minX;
                    glyph.info.bearingY = atlas.ascent - maxY;

                    if (maxX > minX && maxY > minY) {
                        glyph.image.reset(TTF_GetGlyphImage(font, codepoint, nullptr));
                    }

                    if (glyph.image) {
                        const int width = glyph.image->w;
                        const int height = glyph.image->h;
                        if (penX + width > ATLAS_WIDTH) {
                            penX = 0;
                            penY += rowHeight + GLYPH_PADDING;
                            rowHeight = 0;
                        }
                        glyph.position = {penX, penY};
                        glyph.info.width = width;
                        glyph.info.height = height;

                        penX += width + GLYPH_PADDING;
                        rowHeight = std::max(rowHeight, height);
                    }

                    rendered.push_back(std::move(glyph));
                }
            }

            const int atlasHeight = std::max(1, penY + rowHeight);

            const SDL::UniqueSDLSurface atlasSurface{SDL_CreateSurface(ATLAS_WIDTH, atlasHeight, SDL_PIXELFORMAT_RGBA32)};
            if (!atlasSurface) throw std::runtime_error("[FONT] Failed to create the atlas surface.");
            SDL_FillSurfaceRect(atlasSurface.get(), nullptr, SDL_MapSurfaceRGBA(atlasSurface.get(), 0, 0, 0, 0));

            constexpr auto atlasWidthF = static_cast<float>(ATLAS_WIDTH);
            const auto atlasHeightF = static_cast<float>(atlasHeight);

            for (auto&[codepoint, info, image, position] : rendered) {
                if (image) {
                    SDL_Rect destination{position.x, position.y, info.width, info.height};
                    SDL_SetSurfaceBlendMode(image.get(), SDL_BLENDMODE_NONE);
                    SDL_BlitSurface(image.get(), nullptr, atlasSurface.get(), &destination);

                    info.uMin = static_cast<float>(position.x) / atlasWidthF;
                    info.vMin = static_cast<float>(position.y) / atlasHeightF;
                    info.uMax = static_cast<float>(position.x + info.width) / atlasWidthF;
                    info.vMax = static_cast<float>(position.y + info.height) / atlasHeightF;
                }
                atlas.glyphs.emplace(codepoint, info);
            }

            atlas.texture = std::make_unique<Texture>(atlasSurface.get());
            return atlas;
        }


        using FontOpener = std::function<SDL::UniqueTTFFont(float pointSize)>;

        [[nodiscard]] std::unordered_map<FontSize, FontAtlas> LoadAtlases(const FontOpener& open, const std::string_view source) {
            std::unordered_map<FontSize, FontAtlas> atlases;
            for (const auto size : FONT_SIZES) {
                const auto font = open(static_cast<float>(size));
                if (!font) {
                    throw std::runtime_error(std::format("[FONT] Failed to load font {} of size {}: {}", source, static_cast<int>(size), SDL_GetError()));
                }
                atlases.emplace(size, GenerateAtlas(font.get()));
            }
            return atlases;
        }
    }

    FontAtlas::FontAtlas() = default;
    FontAtlas::~FontAtlas() = default;
    FontAtlas::FontAtlas(FontAtlas&&) noexcept = default;
    FontAtlas& FontAtlas::operator=(FontAtlas&&) noexcept = default;

    Font::Font()
        : mAtlases(LoadAtlases([](const float pointSize) {
              return SDL::Helpers::CreateTTFFont(std::span{Embedded::DefaultFontData, Embedded::DefaultFontDataSize}, pointSize);
          }, "<embedded default font>")) {}

    Font::Font(const std::filesystem::path &fontPath)
        : mAtlases(LoadAtlases([&fontPath](const float pointSize) {
              return SDL::Helpers::CreateTTFFont(fontPath, pointSize);
          }, fontPath.string())) {}

    Font::~Font() = default;

    const FontAtlas* Font::GetAtlas(const FontSize size) const noexcept {
        if (const auto it = mAtlases.find(size); it != mAtlases.end()) {
            return &it->second;
        }
        return nullptr;
    }
}
