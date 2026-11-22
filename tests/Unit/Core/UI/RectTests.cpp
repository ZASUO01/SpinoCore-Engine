// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <cstddef>
#include <memory>
#include <gtest/gtest.h>
#include "Core/Rendering/Drawer/DrawCommands.h"
#include "SpinoCore/Core/UI/Elements/Element.h"
#include "SpinoCore/Core/UI/Elements/Rect.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Core::Rendering::UIDrawCommand;
    using SpinoCore::Core::Rendering::UIDrawList;
    using SpinoCore::Core::UI::Elements::Element;
    using SpinoCore::Core::UI::Elements::Rect;
    using SpinoCore::Core::UI::Elements::RectInput;
    using SpinoCore::Math::Vector2f;
    using SpinoCore::Math::Vector4f;

    [[nodiscard]] UIDrawCommand CommandOf(Element& element, const int layer = 1) {
        UIDrawList drawList;
        element.GetDrawCommand(drawList, layer);
        EXPECT_EQ(drawList.commands.size(), 1u);
        return drawList.commands.empty() ? UIDrawCommand{} : drawList.commands.front();
    }

    [[nodiscard]] RectInput WithOrder(const int drawOrder) {
        RectInput input;
        input.drawOrder = drawOrder;
        return input;
    }

    TEST(RectTests, TakesItsColorAndOpacityFromTheInput) {
        RectInput input;
        input.backgroundColor = Vector4f{0.2f, 0.4f, 0.6f, 1.0f};
        input.opacity = 0.5f;

        const Rect rect{input};

        EXPECT_TRUE(Vector4f::NearlyEqual(rect.GetBackgroundColor(), Vector4f{0.2f, 0.4f, 0.6f, 1.0f}));
        EXPECT_FLOAT_EQ(rect.GetOpacity(), 0.5f);
    }

    TEST(RectTests, OpacityFromTheInputIsClampedLikeTheSetter) {
        RectInput input;
        input.opacity = 3.0f;

        EXPECT_FLOAT_EQ(Rect{input}.GetOpacity(), 1.0f);
    }

    TEST(RectTests, SettersClampOpacityAndRadius) {
        Rect rect;

        rect.SetOpacity(-1.0f);
        rect.SetBorderRadius(-4.0f);
        EXPECT_FLOAT_EQ(rect.GetOpacity(), 0.0f);
        EXPECT_FLOAT_EQ(rect.GetBorderRadius(), 0.0f);

        rect.SetOpacity(0.25f);
        rect.SetBorderRadius(12.0f);
        EXPECT_FLOAT_EQ(rect.GetOpacity(), 0.25f);
        EXPECT_FLOAT_EQ(rect.GetBorderRadius(), 12.0f);
    }

    TEST(RectTests, TheCommandColorIsTheBackgroundFadedByTheOpacity) {
        Rect rect;
        rect.SetBackgroundColor(Vector4f{1.0f, 0.5f, 0.0f, 0.8f});
        rect.SetOpacity(0.5f);

        const UIDrawCommand command = CommandOf(rect);

        EXPECT_TRUE(Vector4f::NearlyEqual(command.color, Vector4f{1.0f, 0.5f, 0.0f, 0.4f}));
    }

    TEST(RectTests, TheCommandCarriesTheElementTransformAndRadius) {
        RectInput input;
        input.anchor = Vector2f{50.0f, 60.0f};
        input.size = Vector2f{30.0f, 10.0f};
        Rect rect{input};
        rect.SetBorderRadius(4.0f);

        const UIDrawCommand command = CommandOf(rect);

        for (std::size_t i = 0; i < 16; ++i) EXPECT_FLOAT_EQ(command.transform.data[i], rect.GetTransform().data[i]);
        EXPECT_FLOAT_EQ(command.borderRadius, 4.0f);
    }

    TEST(RectTests, TheCommandSizeIsTheSizeOnScreenIncludingParentScale) {
        RectInput parentInput;
        parentInput.scale = 2.0f;
        Rect parent{parentInput};
        RectInput childInput;
        childInput.size = Vector2f{30.0f, 10.0f};
        childInput.scale = 1.5f;
        Element& child = parent.AddChild(std::make_unique<Rect>(childInput));

        const UIDrawCommand command = CommandOf(child);

        EXPECT_TRUE(Vector2f::NearlyEqual(command.size, Vector2f{90.0f, 30.0f}));
    }

    TEST(RectTests, AHigherDrawOrderSortsLaterWithinALayer) {
        Rect back{WithOrder(10)};
        Rect front{WithOrder(20)};

        EXPECT_LT(CommandOf(back, 1).key, CommandOf(front, 1).key);
    }

    TEST(RectTests, AnythingOnAHigherLayerSortsAfterEverythingBelowIt) {
        Rect lowLayerTop{WithOrder(1000)};
        Rect highLayerBottom{WithOrder(1)};

        EXPECT_LT(CommandOf(lowLayerTop, 1).key, CommandOf(highLayerBottom, 2).key);
    }

    TEST(RectTests, DifferentLayersAndOrdersNeverShareAKey) {
        // With a key of order * layer, (layer 2, order 50) and (layer 1, order 100) would collide.
        Rect a{WithOrder(50)};
        Rect b{WithOrder(100)};

        EXPECT_NE(CommandOf(a, 2).key, CommandOf(b, 1).key);
    }

    TEST(RectTests, StartsWithoutABorder) {
        const Rect rect;

        EXPECT_FLOAT_EQ(rect.GetBorderWidth(), 0.0f);
    }

    TEST(RectTests, ANegativeBorderWidthBecomesZero) {
        Rect rect;

        rect.SetBorder(-2.0f, Vector4f{1.0f, 1.0f, 1.0f, 1.0f});

        EXPECT_FLOAT_EQ(rect.GetBorderWidth(), 0.0f);
    }

    TEST(RectTests, TheBorderGoesIntoTheDrawCommand) {
        Rect rect;
        rect.SetBorder(3.0f, Vector4f{0.0f, 0.0f, 1.0f, 1.0f});

        const UIDrawCommand command = CommandOf(rect);

        EXPECT_FLOAT_EQ(command.borderWidth, 3.0f);
        EXPECT_TRUE(Vector4f::NearlyEqual(command.borderColor, Vector4f{0.0f, 0.0f, 1.0f, 1.0f}));
    }

    TEST(RectTests, TheBorderFadesWithTheOpacity) {
        Rect rect;
        rect.SetOpacity(0.5f);
        rect.SetBorder(3.0f, Vector4f{0.0f, 0.0f, 1.0f, 1.0f});

        const UIDrawCommand command = CommandOf(rect);

        EXPECT_FLOAT_EQ(command.borderColor.w, 0.5f);
    }
}
