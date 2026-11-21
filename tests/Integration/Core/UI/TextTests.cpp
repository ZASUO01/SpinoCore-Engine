// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <algorithm>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include "../../Graphics/GLTestContext.h"
#include "Assets/AssetCache.h"
#include "Core/Rendering/Drawer/DrawCommands.h"
#include "Core/Rendering/Drawer/Drawer.h"
#include "Graphics/Resources/Font.h"
#include "Graphics/Resources/Texture.h"
#include "SpinoCore/Core/UI/Elements/Text.h"
#include "SpinoCore/Graphics/Resources/FontSize.h"
#include "SpinoCore/Math/MatrixFactory.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Assets::AssetCache;
    using SpinoCore::Core::Rendering::Drawer;
    using SpinoCore::Core::Rendering::UIDrawCommand;
    using SpinoCore::Core::Rendering::UIDrawList;
    using SpinoCore::Core::UI::Elements::Text;
    using SpinoCore::Core::UI::Elements::TextAlignment;
    using SpinoCore::Core::UI::Elements::TextInput;
    using SpinoCore::Core::UI::Elements::TextOverflow;
    using SpinoCore::Graphics::Resources::Font;
    using SpinoCore::Graphics::Resources::FontAtlas;
    using SpinoCore::Graphics::Resources::FontSize;
    using SpinoCore::Graphics::Resources::GlyphInfo;
    using SpinoCore::Math::Vector2f;
    using SpinoCore::Math::Vector4f;
    using SpinoCore::Tests::GLTestContext;
    using SpinoCore::Tests::Pixel;
    namespace MatrixFactory = SpinoCore::Math::MatrixFactory;

    constexpr Pixel Red{255, 0, 0, 255};
    constexpr Pixel Blue{0, 0, 255, 255};
    constexpr Pixel Clear{0, 0, 0, 0};
    constexpr Vector4f RedColor{1.0f, 0.0f, 0.0f, 1.0f};
    constexpr Vector4f BlueColor{0.0f, 0.0f, 1.0f, 1.0f};
    constexpr Vector4f Transparent{0.0f, 0.0f, 0.0f, 0.0f};

    // Texts drawn with the engine's default font, both as raw commands and through the real Drawer and shader.
    class TextTests : public GLTestContext {
    protected:
        void SetUp() override {
            GLTestContext::SetUp();
            if (HasFatalFailure()) return;
            drawer.emplace();
            fonts.emplace(std::make_unique<const Font>());
            drawer->GetUIDrawList().fonts = &*fonts;
        }

        void TearDown() override {
            fonts.reset();
            drawer.reset();
            GLTestContext::TearDown();
        }

        [[nodiscard]] static TextInput MakeInput(const std::string& text) {
            TextInput input{};
            input.anchor = Vector2f{100.0f, 50.0f};
            input.size = Vector2f{120.0f, 40.0f};
            input.text = text;
            input.fontSize = FontSize::Size16;
            input.color = RedColor;
            input.backgroundColor = BlueColor;
            input.drawOrder = 5;
            return input;
        }

        [[nodiscard]] std::vector<UIDrawCommand> Commands(Text& text) const {
            UIDrawList list{};
            list.fonts = &*fonts;
            text.GetDrawCommand(list, 1);
            return list.commands;
        }

        [[nodiscard]] static std::vector<UIDrawCommand> Glyphs(const std::vector<UIDrawCommand>& commands) {
            std::vector<UIDrawCommand> glyphs;
            std::ranges::copy_if(commands, std::back_inserter(glyphs), [](const auto& c) { return c.tintTexture; });
            return glyphs;
        }

        [[nodiscard]] static std::vector<UIDrawCommand> Backgrounds(const std::vector<UIDrawCommand>& commands) {
            std::vector<UIDrawCommand> backgrounds;
            std::ranges::copy_if(commands, std::back_inserter(backgrounds), [](const auto& c) { return !c.tintTexture; });
            return backgrounds;
        }

        [[nodiscard]] static float Left(const UIDrawCommand& command) { return command.transform(0, 3); }
        [[nodiscard]] static float Top(const UIDrawCommand& command) { return command.transform(1, 3); }
        [[nodiscard]] static float Width(const UIDrawCommand& command) { return command.transform(0, 0); }

        [[nodiscard]] const FontAtlas& DefaultAtlas(const FontSize size = FontSize::Size16) const {
            return *fonts->GetDefault().GetAtlas(size);
        }

        void Draw(Text& text) {
            text.GetDrawCommand(drawer->GetUIDrawList(), 1);
            const auto size = static_cast<float>(TargetSize);
            drawer->DrawUI(MatrixFactory::CreateOrtho(0.0f, size, size, 0.0f, -1.0f, 1.0f));
        }

        [[nodiscard]] static Pixel UIPixel(const int x, const int y) { return ReadPixel(x, TargetSize - 1 - y); }

        std::optional<Drawer> drawer{};
        std::optional<AssetCache<Font>> fonts{};
    };

    // ---------------------------------------------------------------- commands

    TEST_F(TextTests, SendsOneCommandPerVisibleGlyph) {
        Text text{MakeInput("a b")};

        const auto glyphs = Glyphs(Commands(text));

        EXPECT_EQ(glyphs.size(), 2u);
    }

    TEST_F(TextTests, SendsOneBackgroundCommand) {
        Text text{MakeInput("abc")};

        const auto backgrounds = Backgrounds(Commands(text));

        ASSERT_EQ(backgrounds.size(), 1u);
        EXPECT_EQ(backgrounds[0].color.z, 1.0f);
    }

    TEST_F(TextTests, GlyphsAreTintedWithTheTextColorFromTheFontAtlas) {
        Text text{MakeInput("a")};

        const auto glyph = Glyphs(Commands(text)).at(0);

        EXPECT_EQ(glyph.textureId, DefaultAtlas().texture->GetID());
        EXPECT_EQ(glyph.textureMix, 1.0f);
        EXPECT_EQ(glyph.color.x, 1.0f);
        EXPECT_EQ(glyph.color.z, 0.0f);
    }

    TEST_F(TextTests, AccentedLettersUseTheirOwnGlyph) {
        Text text{MakeInput("ç")};
        const GlyphInfo& expected = DefaultAtlas().glyphs.at(U'ç');

        const auto glyph = Glyphs(Commands(text)).at(0);

        EXPECT_FLOAT_EQ(glyph.textureRect.x, expected.uMin);
        EXPECT_FLOAT_EQ(glyph.textureRect.y, expected.vMin);
        EXPECT_FLOAT_EQ(Width(glyph), static_cast<float>(expected.width));
    }

    TEST_F(TextTests, InvalidUTF8IsDrawnAsAQuestionMark) {
        Text text{MakeInput(std::string{"\xC3"})};
        const GlyphInfo& question = DefaultAtlas().glyphs.at(U'?');

        const auto glyph = Glyphs(Commands(text)).at(0);

        EXPECT_FLOAT_EQ(glyph.textureRect.x, question.uMin);
    }

    TEST_F(TextTests, TheFontSizePicksTheAtlas) {
        auto input = MakeInput("a");
        input.fontSize = FontSize::Size48;
        Text text{input};

        const auto glyph = Glyphs(Commands(text)).at(0);

        EXPECT_EQ(glyph.textureId, DefaultAtlas(FontSize::Size48).texture->GetID());
        EXPECT_FLOAT_EQ(Width(glyph), static_cast<float>(DefaultAtlas(FontSize::Size48).glyphs.at(U'a').width));
    }

    TEST_F(TextTests, TheBackgroundSortsUnderTheGlyphs) {
        Text text{MakeInput("a")};
        const auto commands = Commands(text);

        const auto glyph = Glyphs(commands).at(0);
        const auto background = Backgrounds(commands).at(0);

        EXPECT_LT(background.key, glyph.key);
    }

    TEST_F(TextTests, TheBackgroundFollowsTheTextSize) {
        Text text{MakeInput("a")};

        text.SetSize(Vector2f{200.0f, 60.0f});
        const auto background = Backgrounds(Commands(text)).at(0);

        EXPECT_FLOAT_EQ(background.size.x, 200.0f);
        EXPECT_FLOAT_EQ(background.size.y, 60.0f);
    }

    // ---------------------------------------------------------------- overflow

    TEST_F(TextTests, HiddenOverflowDrawsOnlyTheGlyphsInsideTheBox) {
        auto input = MakeInput(std::string(40, 'a'));
        input.overflow = TextOverflow::Hidden;
        Text text{input};

        const auto glyphs = Glyphs(Commands(text));

        // The box is 120 wide centered at x = 100.
        EXPECT_TRUE(std::ranges::all_of(glyphs, [](const auto& g) { return Left(g) >= 40.0f && Left(g) + Width(g) <= 160.0f; }));
    }

    TEST_F(TextTests, HiddenOverflowDropsTheGlyphsThatDoNotFit) {
        auto input = MakeInput(std::string(40, 'a'));
        input.overflow = TextOverflow::Hidden;
        Text text{input};

        const auto glyphs = Glyphs(Commands(text));

        EXPECT_LT(glyphs.size(), 40u);
    }

    TEST_F(TextTests, ScrollToEndShowsTheLastCharacterOfALongLine) {
        auto input = MakeInput(std::string(40, 'a') + "b");
        input.alignment = TextAlignment::Left;
        input.overflow = TextOverflow::ScrollToEnd;
        Text text{input};

        const auto last = Glyphs(Commands(text)).back();

        EXPECT_FLOAT_EQ(last.textureRect.x, DefaultAtlas().glyphs.at(U'b').uMin);
    }

    TEST_F(TextTests, ScrollToEndKeepsTheAlignmentOfALineThatFits) {
        auto plainInput = MakeInput("ab");
        plainInput.alignment = TextAlignment::Left;
        auto scrolledInput = plainInput;
        scrolledInput.overflow = TextOverflow::ScrollToEnd;
        Text plain{plainInput};
        Text scrolled{scrolledInput};

        const auto plainGlyph = Glyphs(Commands(plain)).at(0);
        const auto scrolledGlyph = Glyphs(Commands(scrolled)).at(0);

        EXPECT_FLOAT_EQ(Left(scrolledGlyph), Left(plainGlyph));
    }

    // ---------------------------------------------------------------- layout

    TEST_F(TextTests, GlyphsAdvanceFromLeftToRight) {
        Text text{MakeInput("ab")};

        const auto glyphs = Glyphs(Commands(text));

        EXPECT_LT(Left(glyphs.at(0)), Left(glyphs.at(1)));
    }

    TEST_F(TextTests, CenteredTextIsCenteredOnTheElement) {
        Text text{MakeInput("HH")};
        const auto glyphs = Glyphs(Commands(text));

        const float left = Left(glyphs.front());
        const float right = Left(glyphs.back()) + Width(glyphs.back());

        EXPECT_NEAR((left + right) / 2.0f, 100.0f, 2.0f);
    }

    TEST_F(TextTests, LeftAlignedTextStartsAtTheLeftEdge) {
        auto input = MakeInput("H");
        input.alignment = TextAlignment::Left;
        Text text{input};
        const GlyphInfo& h = DefaultAtlas().glyphs.at(U'H');

        const auto glyph = Glyphs(Commands(text)).at(0);

        EXPECT_FLOAT_EQ(Left(glyph), 40.0f + static_cast<float>(h.bearingX));
    }

    TEST_F(TextTests, RightAlignedTextEndsAtTheRightEdge) {
        auto input = MakeInput("H");
        input.alignment = TextAlignment::Right;
        Text text{input};
        const GlyphInfo& h = DefaultAtlas().glyphs.at(U'H');

        const auto glyph = Glyphs(Commands(text)).at(0);

        EXPECT_FLOAT_EQ(Left(glyph), 160.0f - static_cast<float>(h.advance) + static_cast<float>(h.bearingX));
    }

    TEST_F(TextTests, NewLinesStartALineBelow) {
        Text text{MakeInput("H\nH")};

        const auto glyphs = Glyphs(Commands(text));

        ASSERT_EQ(glyphs.size(), 2u);
        EXPECT_FLOAT_EQ(Top(glyphs.at(1)) - Top(glyphs.at(0)), static_cast<float>(DefaultAtlas().lineHeight));
    }

    TEST_F(TextTests, TheElementScaleScalesTheGlyphs) {
        auto input = MakeInput("H");
        input.scale = 2.0f;
        Text text{input};

        const auto glyph = Glyphs(Commands(text)).at(0);

        EXPECT_FLOAT_EQ(Width(glyph), 2.0f * static_cast<float>(DefaultAtlas().glyphs.at(U'H').width));
    }

    TEST_F(TextTests, SetTextLaysTheNewTextOut) {
        Text text{MakeInput("a")};
        (void)Commands(text);

        text.SetText("abc");

        EXPECT_EQ(Glyphs(Commands(text)).size(), 3u);
    }

    // ---------------------------------------------------------------- fonts

    TEST_F(TextTests, UnknownFontsDrawTheDefaultOne) {
        auto input = MakeInput("a");
        input.font = "not-loaded";
        Text text{input};

        const auto glyph = Glyphs(Commands(text)).at(0);

        EXPECT_EQ(glyph.textureId, DefaultAtlas().texture->GetID());
    }

    TEST_F(TextTests, WithoutAFontNameTheDefaultFontIsDrawn) {
        Text text{MakeInput("a")};

        const auto glyph = Glyphs(Commands(text)).at(0);

        EXPECT_EQ(glyph.textureId, DefaultAtlas().texture->GetID());
    }

    TEST_F(TextTests, FontsCachedLaterAreUsedFromTheNextFrame) {
        auto input = MakeInput("a");
        input.font = "custom";
        Text text{input};
        (void)Commands(text);

        const Font& custom = fonts->Add("custom", std::make_unique<const Font>());
        const auto glyph = Glyphs(Commands(text)).at(0);

        EXPECT_EQ(glyph.textureId, custom.GetAtlas(FontSize::Size16)->texture->GetID());
    }

    TEST_F(TextTests, WithoutAFontCacheOnlyTheBackgroundIsSent) {
        Text text{MakeInput("abc")};
        UIDrawList list{};

        text.GetDrawCommand(list, 1);

        ASSERT_EQ(list.commands.size(), 1u);
        EXPECT_FALSE(list.commands[0].tintTexture);
    }

    // ---------------------------------------------------------------- drawing

    TEST_F(TextTests, GlyphsAreDrawnInTheTextColorOverTheBackground) {
        TextInput input{};
        input.anchor = Vector2f{8.0f, 8.0f};
        input.size = Vector2f{16.0f, 16.0f};
        input.text = "|";
        input.fontSize = FontSize::Size72;
        input.color = RedColor;
        input.backgroundColor = BlueColor;
        Text text{input};

        Draw(text);

        EXPECT_EQ(UIPixel(8, 8), Red);
        EXPECT_EQ(UIPixel(0, 8), Blue);
    }

    TEST_F(TextTests, ATransparentBackgroundLeavesOnlyTheGlyphs) {
        TextInput input{};
        input.anchor = Vector2f{8.0f, 8.0f};
        input.size = Vector2f{16.0f, 16.0f};
        input.text = "|";
        input.fontSize = FontSize::Size72;
        input.color = RedColor;
        input.backgroundColor = Transparent;
        Text text{input};

        Draw(text);

        EXPECT_EQ(UIPixel(8, 8), Red);
        EXPECT_EQ(UIPixel(0, 8), Clear);
    }
}
