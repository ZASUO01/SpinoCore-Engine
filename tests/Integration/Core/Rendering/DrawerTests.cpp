// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <cstdint>
#include <optional>
#include <gtest/gtest.h>
#include "../../Graphics/GLTestContext.h"
#include "Core/Rendering/Drawer/DrawCommands.h"
#include "Core/Rendering/Drawer/Drawer.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/MatrixFactory.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Core::Rendering::Drawer;
    using SpinoCore::Core::Rendering::UIDrawCommand;
    using SpinoCore::Math::Matrix4f;
    using SpinoCore::Math::Vector2f;
    using SpinoCore::Math::Vector3f;
    using SpinoCore::Math::Vector4f;
    using SpinoCore::Tests::GLTestContext;
    using SpinoCore::Tests::Pixel;
    namespace MatrixFactory = SpinoCore::Math::MatrixFactory;

    constexpr Pixel Black{0, 0, 0, 0};
    constexpr Pixel Red{255, 0, 0, 255};
    constexpr Pixel Blue{0, 0, 255, 255};
    constexpr Vector4f RedColor{1.0f, 0.0f, 0.0f, 1.0f};
    constexpr Vector4f BlueColor{0.0f, 0.0f, 1.0f, 1.0f};

    class DrawerTests : public GLTestContext {
    protected:
        void SetUp() override {
            GLTestContext::SetUp();
            drawer.emplace();
        }

        void TearDown() override {
            drawer.reset();
            GLTestContext::TearDown();
        }

        // Same convention as the engine: window pixels, origin at the top-left corner, y pointing down.
        [[nodiscard]] static Matrix4f PixelOrtho() {
            const auto size = static_cast<float>(TargetSize);
            return MatrixFactory::CreateOrtho(0.0f, size, size, 0.0f, -1.0f, 1.0f);
        }

        // A command for an axis-aligned rectangle given in pixels, built the way elements build theirs:
        // the unit quad is centered and scaled to the rectangle.
        [[nodiscard]] static UIDrawCommand RectCommand(const float x, const float y, const float width, const float height,
                                                       const Vector4f& color, const std::uint64_t key = 0) {
            UIDrawCommand command{};
            command.key = key;
            command.transform = MatrixFactory::CreateTranslation(Vector3f{x + width / 2.0f, y + height / 2.0f, 0.0f}) *
                                MatrixFactory::CreateScale(width, height, 1.0f) *
                                MatrixFactory::CreateTranslation(Vector3f{-0.5f, -0.5f, 0.0f});
            command.color = color;
            command.size = Vector2f{width, height};
            return command;
        }

        // Reads a pixel addressed like the UI: from the top-left corner, y pointing down.
        [[nodiscard]] static Pixel UIPixel(const int x, const int y) { return ReadPixel(x, TargetSize - 1 - y); }

        void Submit(const UIDrawCommand& command) { drawer->GetUIDrawList().commands.push_back(command); }

        std::optional<Drawer> drawer{};
    };

    TEST_F(DrawerTests, DrawsACommandWhereItsTransformPlacesItInPixels) {
        Submit(RectCommand(0.0f, 0.0f, 8.0f, 16.0f, RedColor));

        drawer->DrawUI(PixelOrtho());

        EXPECT_EQ(UIPixel(2, 8), Red);
        EXPECT_EQ(UIPixel(12, 8), Black);
    }

    TEST_F(DrawerTests, TheTopOfTheUIIsTheTopOfTheScreen) {
        Submit(RectCommand(0.0f, 0.0f, 16.0f, 4.0f, RedColor));

        drawer->DrawUI(PixelOrtho());

        EXPECT_EQ(UIPixel(8, 1), Red);
        EXPECT_EQ(UIPixel(8, 14), Black);
    }

    TEST_F(DrawerTests, CommandsWithAHigherKeyAreDrawnOnTop) {
        Submit(RectCommand(0.0f, 0.0f, 16.0f, 16.0f, BlueColor, 2));
        Submit(RectCommand(0.0f, 0.0f, 16.0f, 16.0f, RedColor, 1));

        drawer->DrawUI(PixelOrtho());

        EXPECT_EQ(UIPixel(8, 8), Blue);
    }

    TEST_F(DrawerTests, CommandsWithTheSameKeyKeepTheOrderTheyWereSubmittedIn) {
        // Many ties mixed with lower keys: an unstable sort moves equal elements around while partitioning.
        for (int i = 0; i < 256; ++i) {
            const std::uint64_t key = i % 3 == 0 ? 1 : 5;
            Submit(RectCommand(0.0f, 0.0f, 16.0f, 16.0f, i % 2 == 0 ? BlueColor : RedColor, key));
        }
        // The last command submitted with the top key must be the one left on screen.
        Submit(RectCommand(0.0f, 0.0f, 16.0f, 16.0f, RedColor, 5));

        drawer->DrawUI(PixelOrtho());

        EXPECT_EQ(UIPixel(8, 8), Red);
    }

    TEST_F(DrawerTests, TranslucentColorsBlendOverWhatIsBehind) {
        Submit(RectCommand(0.0f, 0.0f, 16.0f, 16.0f, BlueColor, 1));
        Submit(RectCommand(0.0f, 0.0f, 16.0f, 16.0f, Vector4f{1.0f, 0.0f, 0.0f, 0.5f}, 2));

        drawer->DrawUI(PixelOrtho());

        const Pixel mixed = UIPixel(8, 8);
        EXPECT_NEAR(mixed[0], 128, 2);
        EXPECT_NEAR(mixed[2], 128, 2);
    }

    TEST_F(DrawerTests, ABorderRadiusRoundsTheCorners) {
        UIDrawCommand command = RectCommand(0.0f, 0.0f, 16.0f, 16.0f, RedColor);
        command.borderRadius = 8.0f;
        Submit(command);

        drawer->DrawUI(PixelOrtho());

        EXPECT_EQ(UIPixel(0, 0)[3], 0);
        EXPECT_EQ(UIPixel(8, 8), Red);
    }

    // The engine refills the list every frame, so drawing must consume it or it grows forever.
    TEST_F(DrawerTests, DrawingConsumesTheQueuedCommands) {
        Submit(RectCommand(0.0f, 0.0f, 4.0f, 4.0f, RedColor, 2));
        Submit(RectCommand(0.0f, 0.0f, 4.0f, 4.0f, RedColor, 1));

        drawer->DrawUI(PixelOrtho());

        EXPECT_TRUE(drawer->GetUIDrawList().commands.empty());
    }

    TEST_F(DrawerTests, ABorderIsDrawnAlongTheEdges) {
        UIDrawCommand command = RectCommand(0.0f, 0.0f, 16.0f, 16.0f, RedColor);
        command.borderWidth = 3.0f;
        command.borderColor = BlueColor;
        Submit(command);

        drawer->DrawUI(PixelOrtho());

        EXPECT_EQ(UIPixel(1, 8), Blue);
    }

    TEST_F(DrawerTests, TheInsideOfABorderedRectKeepsItsColor) {
        UIDrawCommand command = RectCommand(0.0f, 0.0f, 16.0f, 16.0f, RedColor);
        command.borderWidth = 3.0f;
        command.borderColor = BlueColor;
        Submit(command);

        drawer->DrawUI(PixelOrtho());

        EXPECT_EQ(UIPixel(8, 8), Red);
    }

    TEST_F(DrawerTests, ABorderFollowsTheRoundedCorners) {
        UIDrawCommand command = RectCommand(0.0f, 0.0f, 16.0f, 16.0f, RedColor);
        command.borderRadius = 8.0f;
        command.borderWidth = 3.0f;
        command.borderColor = BlueColor;
        Submit(command);

        drawer->DrawUI(PixelOrtho());

        EXPECT_EQ(UIPixel(3, 3), Blue);
    }

    TEST_F(DrawerTests, DrawingAnEmptyListDrawsNothing) {
        drawer->DrawUI(PixelOrtho());

        EXPECT_EQ(UIPixel(8, 8), Black);
    }
}
