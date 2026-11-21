// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <cstddef>
#include <gtest/gtest.h>
#include "Assets/AssetCache.h"
#include "Core/Rendering/Drawer/DrawCommands.h"
#include "Graphics/Resources/Texture.h"
#include "SpinoCore/Core/UI/Elements/Image.h"
#include "SpinoCore/Core/UI/Elements/Rect.h"
#include "SpinoCore/Math/Vector.h"

// No GL context here: only what an Image sends to the renderer when no texture can be found.
namespace {
    using SpinoCore::Assets::AssetCache;
    using SpinoCore::Core::Rendering::UIDrawCommand;
    using SpinoCore::Core::Rendering::UIDrawList;
    using SpinoCore::Core::UI::Elements::Image;
    using SpinoCore::Core::UI::Elements::ImageInput;
    using SpinoCore::Core::UI::Elements::Rect;
    using SpinoCore::Core::UI::Elements::RectInput;
    using SpinoCore::Graphics::Resources::Texture;
    using SpinoCore::Math::Vector2f;
    using SpinoCore::Math::Vector4f;

    [[nodiscard]] ImageInput MakeInput(const char* name) {
        ImageInput input{};
        input.anchor = Vector2f{40.0f, 30.0f};
        input.size = Vector2f{20.0f, 10.0f};
        input.scale = 2.0f;
        input.drawOrder = 7;
        input.backgroundColor = Vector4f{1.0f, 0.0f, 0.0f, 0.5f};
        input.opacity = 0.4f;
        input.name = name;
        return input;
    }

    [[nodiscard]] UIDrawCommand Draw(Image& image, const UIDrawList& base = {}) {
        UIDrawList list{.commands = {}, .textures = base.textures};
        image.GetDrawCommand(list, 3);
        EXPECT_EQ(list.commands.size(), 1u);
        return list.commands.empty() ? UIDrawCommand{} : list.commands.front();
    }

    TEST(ImageTests, KeepsTheTextureNameItWasCreatedWith) {
        const Image image{MakeInput("hero")};

        EXPECT_EQ(image.GetTexture(), "hero");
    }

    TEST(ImageTests, TheDefaultNameIsDefault) {
        const Image image{ImageInput{}};

        EXPECT_EQ(image.GetTexture(), "default");
    }

    TEST(ImageTests, SetTextureChangesTheName) {
        Image image{MakeInput("hero")};

        image.SetTexture("tree");

        EXPECT_EQ(image.GetTexture(), "tree");
    }

    TEST(ImageTests, WithoutATextureCacheOnlyTheBackgroundIsDrawn) {
        Image image{MakeInput("hero")};

        const UIDrawCommand command = Draw(image);

        EXPECT_EQ(command.textureId, 0u);
        EXPECT_EQ(command.textureMix, 0.0f);
    }

    TEST(ImageTests, WithoutTheTextureOrADefaultOnlyTheBackgroundIsDrawn) {
        const AssetCache<Texture> textures{};
        Image image{MakeInput("hero")};

        const UIDrawCommand command = Draw(image, UIDrawList{.commands = {}, .textures = &textures});

        EXPECT_EQ(command.textureId, 0u);
        EXPECT_EQ(command.textureMix, 0.0f);
    }

    // The texture is laid over the background first and the opacity applies to the result,
    // so the background keeps its own alpha and the opacity travels on its own.
    TEST(ImageTests, BackgroundAndOpacityAreSentSeparately) {
        Image image{MakeInput("hero")};

        const UIDrawCommand command = Draw(image);

        EXPECT_EQ(command.color.x, 1.0f);
        EXPECT_EQ(command.color.w, 0.5f);
        EXPECT_FLOAT_EQ(command.opacity, 0.4f);
    }

    TEST(ImageTests, PlacementMatchesARectWithTheSameInput) {
        const ImageInput input = MakeInput("hero");
        Image image{input};
        Rect rect{static_cast<const RectInput&>(input)};
        image.SetBorderRadius(4.0f);
        rect.SetBorderRadius(4.0f);

        const UIDrawCommand imageCommand = Draw(image);
        UIDrawList rectList{};
        rect.GetDrawCommand(rectList, 3);
        const UIDrawCommand& rectCommand = rectList.commands.front();

        EXPECT_EQ(imageCommand.key, rectCommand.key);
        EXPECT_EQ(imageCommand.key, SpinoCore::Core::Rendering::MakeUIDrawKey(3, 7, 0));
        EXPECT_EQ(imageCommand.size.x, rectCommand.size.x);
        EXPECT_EQ(imageCommand.size.y, rectCommand.size.y);
        EXPECT_EQ(imageCommand.borderRadius, rectCommand.borderRadius);
        for (std::size_t row = 0; row < 4; ++row) {
            for (std::size_t column = 0; column < 4; ++column) {
                EXPECT_FLOAT_EQ(imageCommand.transform(row, column), rectCommand.transform(row, column));
            }
        }
    }
}
