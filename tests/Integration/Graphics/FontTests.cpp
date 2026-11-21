// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <random>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unordered_map>
#include <vector>
#include <gtest/gtest.h>
#include <glad/glad.h>
#include "GLTestContext.h"
#include "Graphics/Resources/EmbeddedFonts.h"
#include "Graphics/Resources/Font.h"
#include "Graphics/Resources/Texture.h"
#include "SpinoCore/Graphics/Resources/FontSize.h"
#include "SpinoCore/Utils/Size2D.h"

namespace {
    namespace fs = std::filesystem;
    using SpinoCore::Graphics::Resources::Font;
    using SpinoCore::Graphics::Resources::FontAtlas;
    using SpinoCore::Graphics::Resources::FontSize;
    using SpinoCore::Graphics::Resources::GlyphInfo;
    using SpinoCore::Tests::Pixel;
    namespace Embedded = SpinoCore::Graphics::Resources::Embedded;

    // A fresh folder under the system temp directory, removed when the test ends.
    class TempDirectory final {
    public:
        TempDirectory() {
            std::random_device rd;
            mPath = fs::temp_directory_path() / ("spinocore-font-" + std::to_string(rd()) + std::to_string(rd()));
            fs::create_directories(mPath);
        }
        ~TempDirectory() {
            std::error_code ignored;
            fs::remove_all(mPath, ignored);
        }
        TempDirectory(const TempDirectory&) = delete;
        TempDirectory& operator=(const TempDirectory&) = delete;

        [[nodiscard]] const fs::path& Path() const { return mPath; }

    private:
        fs::path mPath;
    };

    [[nodiscard]] std::vector<Pixel> ReadTexture(const SpinoCore::Graphics::Resources::Texture& texture) {
        const auto size = texture.GetSize();
        std::vector<Pixel> pixels(static_cast<std::size_t>(size.width * size.height));
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glGetTextureImage(texture.GetID(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
                          static_cast<GLsizei>(pixels.size() * sizeof(Pixel)), pixels.data());
        glPixelStorei(GL_PACK_ALIGNMENT, 4);
        return pixels;
    }

    using FontTests = SpinoCore::Tests::GLTestContext;

    // ---------------------------------------------------------------- default font

    TEST_F(FontTests, DefaultFontBuildsAnAtlasForEverySize) {
        const Font font{};

        for (const auto size : {FontSize::Size10, FontSize::Size12, FontSize::Size16, FontSize::Size18,
                                FontSize::Size24, FontSize::Size30, FontSize::Size36, FontSize::Size42,
                                FontSize::Size48, FontSize::Size54, FontSize::Size60, FontSize::Size72}) {
            const FontAtlas* atlas = font.GetAtlas(size);
            ASSERT_NE(atlas, nullptr) << static_cast<int>(size);
            EXPECT_NE(atlas->texture->GetID(), 0u);
        }
    }

    TEST_F(FontTests, DefaultFontCoversAccentedLatinLetters) {
        const Font font{};
        const FontAtlas* atlas = font.GetAtlas(FontSize::Size16);

        for (const char32_t letter : {U'á', U'ã', U'ç', U'é', U'ô', U'Ç', U'ñ'}) {
            EXPECT_TRUE(atlas->glyphs.contains(letter)) << static_cast<unsigned>(letter);
        }
    }

    TEST_F(FontTests, SpaceAdvancesThePenWithoutTakingAtlasSpace) {
        const Font font{};

        const GlyphInfo& space = font.GetAtlas(FontSize::Size16)->glyphs.at(U' ');

        EXPECT_GT(space.advance, 0);
        EXPECT_EQ(space.width, 0);
        EXPECT_EQ(space.height, 0);
    }

    // At the biggest size the glyphs no longer fit in one 512x512 page, so the atlas grows instead of clipping them.
    TEST_F(FontTests, EveryGlyphFitsInsideItsAtlas) {
        const Font font{};
        const FontAtlas* atlas = font.GetAtlas(FontSize::Size72);
        const auto atlasSize = atlas->texture->GetSize();

        for (const auto& [codepoint, glyph] : atlas->glyphs) {
            EXPECT_LE(glyph.uMax, 1.0f) << static_cast<unsigned>(codepoint);
            EXPECT_LE(glyph.vMax, 1.0f) << static_cast<unsigned>(codepoint);
            EXPECT_NEAR((glyph.uMax - glyph.uMin) * static_cast<float>(atlasSize.width), static_cast<float>(glyph.width), 0.01f);
            EXPECT_NEAR((glyph.vMax - glyph.vMin) * static_cast<float>(atlasSize.height), static_cast<float>(glyph.height), 0.01f);
        }
    }

    // The atlas holds white glyphs whose coverage is in the alpha channel, so the UI shader can tint them.
    TEST_F(FontTests, GlyphPixelsAreWhiteWithCoverageInAlpha) {
        const Font font{};
        const FontAtlas* atlas = font.GetAtlas(FontSize::Size72);
        const GlyphInfo& bar = atlas->glyphs.at(U'|');
        const auto atlasSize = atlas->texture->GetSize();
        const auto pixels = ReadTexture(*atlas->texture);

        const int centerX = static_cast<int>(bar.uMin * static_cast<float>(atlasSize.width)) + bar.width / 2;
        const int centerY = static_cast<int>(bar.vMin * static_cast<float>(atlasSize.height)) + bar.height / 2;
        const Pixel center = pixels[static_cast<std::size_t>(centerY * atlasSize.width + centerX)];

        EXPECT_EQ(center, (Pixel{255, 255, 255, 255}));
    }

    // Measured from the top of the line: a descender like 'g' ends below the baseline that capitals sit on.
    TEST_F(FontTests, BearingYPlacesGlyphsOnACommonBaseline) {
        const Font font{};
        const FontAtlas* atlas = font.GetAtlas(FontSize::Size36);
        const GlyphInfo& capital = atlas->glyphs.at(U'T');
        const GlyphInfo& descender = atlas->glyphs.at(U'g');

        const int capitalBottom = capital.bearingY + capital.height;
        const int descenderBottom = descender.bearingY + descender.height;

        EXPECT_NEAR(capitalBottom, atlas->ascent, 1);
        EXPECT_GT(descenderBottom, atlas->ascent);
    }

    TEST_F(FontTests, BiggerSizesHaveTallerLines) {
        const Font font{};

        EXPECT_LT(font.GetAtlas(FontSize::Size12)->lineHeight, font.GetAtlas(FontSize::Size48)->lineHeight);
    }

    // ---------------------------------------------------------------- from a file

    TEST_F(FontTests, LoadsAFontFileWithTheSameGlyphsAsTheEmbeddedCopy) {
        const TempDirectory dir;
        const auto path = dir.Path() / "inter.otf";
        {
            std::ofstream out(path, std::ios::binary);
            out.write(reinterpret_cast<const char*>(Embedded::DefaultFontData), static_cast<std::streamsize>(Embedded::DefaultFontDataSize));
        }

        const Font fromFile{path};
        const Font embedded{};

        const GlyphInfo& fileGlyph = fromFile.GetAtlas(FontSize::Size24)->glyphs.at(U'ç');
        const GlyphInfo& embeddedGlyph = embedded.GetAtlas(FontSize::Size24)->glyphs.at(U'ç');
        EXPECT_EQ(fileGlyph.advance, embeddedGlyph.advance);
        EXPECT_EQ(fileGlyph.width, embeddedGlyph.width);
        EXPECT_EQ(fileGlyph.height, embeddedGlyph.height);
    }

    TEST_F(FontTests, MissingFileThrows) {
        const TempDirectory dir;

        EXPECT_THROW({ const Font font{dir.Path() / "missing-font.ttf"}; }, std::runtime_error);
    }

    TEST_F(FontTests, FileThatIsNotAFontThrows) {
        const TempDirectory dir;
        const auto path = dir.Path() / "not-a-font.ttf";
        std::ofstream(path) << "definitely not a font";

        EXPECT_THROW({ const Font font{path}; }, std::runtime_error);
    }
}
