// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <filesystem>
#include <memory>
#include <unordered_map>

namespace SpinoCore::Graphics::Resources {
    class Texture;
    enum class FontSize : int;

    struct GlyphInfo final {
        float uMin{0.0f}, vMin{0.0f};
        float uMax{0.0f}, vMax{0.0f};

        int width{0}, height{0};
        int bearingX{0}, bearingY{0};
        int advance{0};
    };

    using GlyphTable = std::unordered_map<char32_t, GlyphInfo>;

    struct FontAtlas final {
        std::unique_ptr<Texture> texture;
        GlyphTable glyphs;
        int ascent{0};
        int lineHeight{0};

        FontAtlas();
        ~FontAtlas();
        FontAtlas(const FontAtlas&) = delete;
        FontAtlas& operator=(const FontAtlas&) = delete;
        FontAtlas(FontAtlas&&) noexcept;
        FontAtlas& operator=(FontAtlas&&) noexcept;
    };

    class Font final {
    public:
        Font();
        explicit Font(const std::filesystem::path& fontPath);
        ~Font();

        Font(const Font&) = delete;
        Font& operator=(const Font&) = delete;
        Font(Font&&) = delete;
        Font& operator=(Font&&) = delete;

        [[nodiscard]] const FontAtlas* GetAtlas(FontSize size) const noexcept;

    private:
        std::unordered_map<FontSize, FontAtlas> mAtlases;
    };
}
