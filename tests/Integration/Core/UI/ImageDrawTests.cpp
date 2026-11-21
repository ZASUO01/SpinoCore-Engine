// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <filesystem>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <system_error>
#include <vector>
#include <gtest/gtest.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <glad/glad.h>
#include "../../Graphics/GLTestContext.h"
#include "Assets/AssetCache.h"
#include "Core/Rendering/Drawer/DrawCommands.h"
#include "Core/Rendering/Drawer/Drawer.h"
#include "Graphics/Resources/Texture.h"
#include "SpinoCore/Core/UI/Elements/Image.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/MatrixFactory.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    namespace fs = std::filesystem;
    using SpinoCore::Assets::AssetCache;
    using SpinoCore::Core::Rendering::Drawer;
    using SpinoCore::Core::UI::Elements::Image;
    using SpinoCore::Core::UI::Elements::ImageInput;
    using SpinoCore::Graphics::Resources::Texture;
    using SpinoCore::Math::Matrix4f;
    using SpinoCore::Math::Vector2f;
    using SpinoCore::Math::Vector4f;
    using SpinoCore::Tests::GLTestContext;
    using SpinoCore::Tests::Pixel;
    namespace MatrixFactory = SpinoCore::Math::MatrixFactory;

    constexpr Pixel Red{255, 0, 0, 255};
    constexpr Pixel Blue{0, 0, 255, 255};
    constexpr Pixel Green{0, 255, 0, 255};
    constexpr Pixel White{255, 255, 255, 255};
    constexpr Pixel Clear{0, 0, 0, 0};
    constexpr Vector4f Transparent{0.0f, 0.0f, 0.0f, 0.0f};
    constexpr Vector4f GreenColor{0.0f, 1.0f, 0.0f, 1.0f};

    // Draws Images through the real Drawer and shader into the 16x16 test framebuffer, with real textures.
    class ImageDrawTests : public GLTestContext {
    protected:
        void SetUp() override {
            GLTestContext::SetUp();
            if (HasFatalFailure()) return;
            drawer.emplace();
            textures.emplace(std::make_unique<const Texture>());
            drawer->GetUIDrawList().textures = &*textures;

            std::random_device rd;
            folder = fs::temp_directory_path() / ("spinocore-image-" + std::to_string(rd()) + std::to_string(rd()));
            fs::create_directories(folder);
        }

        void TearDown() override {
            std::error_code ignored;
            fs::remove_all(folder, ignored);
            // Textures and GL objects go before the context the fixture owns.
            textures.reset();
            drawer.reset();
            GLTestContext::TearDown();
        }

        // A texture of width x height whose pixels come from color(x, y), row 0 at the top.
        template <typename ColorAt>
        void AddTexture(const std::string& name, const int width, const int height, ColorAt color) {
            SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
            ASSERT_NE(surface, nullptr);
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    const Pixel p = color(x, y);
                    SDL_WriteSurfacePixel(surface, x, y, p[0], p[1], p[2], p[3]);
                }
            }
            const fs::path path = folder / (name + ".png");
            const bool saved = IMG_SavePNG(surface, path.string().c_str());
            SDL_DestroySurface(surface);
            ASSERT_TRUE(saved);
            textures->Add(name, std::make_unique<const Texture>(path));
        }

        // Fills the whole 16x16 target.
        [[nodiscard]] static ImageInput FullScreen(const std::string& name, const Vector4f& background = Transparent) {
            ImageInput input{};
            input.anchor = Vector2f{8.0f, 8.0f};
            input.size = Vector2f{16.0f, 16.0f};
            input.backgroundColor = background;
            input.name = name;
            return input;
        }

        void Draw(Image& image) {
            image.GetDrawCommand(drawer->GetUIDrawList(), 1);
            const auto size = static_cast<float>(TargetSize);
            drawer->DrawUI(MatrixFactory::CreateOrtho(0.0f, size, size, 0.0f, -1.0f, 1.0f));
        }

        void ClearTarget() const { glClear(GL_COLOR_BUFFER_BIT); }

        // Reads a pixel addressed like the UI: from the top-left corner, y pointing down.
        [[nodiscard]] static Pixel UIPixel(const int x, const int y) { return ReadPixel(x, TargetSize - 1 - y); }

        std::optional<Drawer> drawer{};
        std::optional<AssetCache<Texture>> textures{};
        fs::path folder;
    };

    TEST_F(ImageDrawTests, DrawsTheCachedTextureWithItsTopRowAtTheTop) {
        AddTexture("flag", 2, 2, [](int, const int y) { return y == 0 ? Red : Blue; });
        Image image{FullScreen("flag")};

        Draw(image);

        EXPECT_EQ(UIPixel(8, 1), Red);
        EXPECT_EQ(UIPixel(8, 14), Blue);
    }

    TEST_F(ImageDrawTests, LeftColumnStaysOnTheLeft) {
        AddTexture("flag", 2, 2, [](const int x, int) { return x == 0 ? Red : Blue; });
        Image image{FullScreen("flag")};

        Draw(image);

        EXPECT_EQ(UIPixel(1, 8), Red);
        EXPECT_EQ(UIPixel(14, 8), Blue);
    }

    TEST_F(ImageDrawTests, MissingTexturesDrawTheDefaultOne) {
        Image image{FullScreen("not-loaded")};

        Draw(image);

        EXPECT_EQ(UIPixel(8, 8), White);
    }

    // Loads finish a few frames after the request: the image has to pick the texture up once it arrives.
    TEST_F(ImageDrawTests, TexturesCachedLaterAreUsedFromTheNextFrame) {
        Image image{FullScreen("hero")};
        Draw(image);
        ASSERT_EQ(UIPixel(8, 8), White);

        AddTexture("hero", 1, 1, [](int, int) { return Red; });
        ClearTarget();
        Draw(image);

        EXPECT_EQ(UIPixel(8, 8), Red);
    }

    TEST_F(ImageDrawTests, RemovedTexturesFallBackToTheDefault) {
        AddTexture("hero", 1, 1, [](int, int) { return Red; });
        Image image{FullScreen("hero")};
        Draw(image);
        ASSERT_EQ(UIPixel(8, 8), Red);

        textures->Remove("hero");
        ClearTarget();
        Draw(image);

        EXPECT_EQ(UIPixel(8, 8), White);
    }

    TEST_F(ImageDrawTests, SetTextureSwitchesTheTextureDrawn) {
        AddTexture("red", 1, 1, [](int, int) { return Red; });
        AddTexture("blue", 1, 1, [](int, int) { return Blue; });
        Image image{FullScreen("red")};

        image.SetTexture("blue");
        Draw(image);

        EXPECT_EQ(UIPixel(8, 8), Blue);
    }

    TEST_F(ImageDrawTests, SetTextureAfterADrawSwitchesTheTextureDrawn) {
        AddTexture("red", 1, 1, [](int, int) { return Red; });
        AddTexture("blue", 1, 1, [](int, int) { return Blue; });
        Image image{FullScreen("red")};
        Draw(image);
        ASSERT_EQ(UIPixel(8, 8), Red);

        image.SetTexture("blue");
        ClearTarget();
        Draw(image);

        EXPECT_EQ(UIPixel(8, 8), Blue);
    }

    // The demo's case: the manifest loads one texture per frame, so an image whose texture comes second
    // is drawn once with the default while another image already has its own.
    TEST_F(ImageDrawTests, AnImageWaitingForItsTextureGetsItWhenItLoadsAfterAnother) {
        AddTexture("hero", 1, 1, [](int, int) { return Red; });
        Image image{FullScreen("image")};
        Draw(image);
        ASSERT_EQ(UIPixel(8, 8), White);

        AddTexture("image", 1, 1, [](int, int) { return Blue; });
        ClearTarget();
        Draw(image);

        EXPECT_EQ(UIPixel(8, 8), Blue);
    }

    TEST_F(ImageDrawTests, TransparentPartsOfTheTextureShowTheBackground) {
        AddTexture("window", 1, 1, [](int, int) { return Clear; });
        Image image{FullScreen("window", GreenColor)};

        Draw(image);

        EXPECT_EQ(UIPixel(8, 8), Green);
    }

    TEST_F(ImageDrawTests, ATransparentBackgroundLeavesOnlyTheTexture) {
        AddTexture("sprite", 2, 1, [](const int x, int) { return x == 0 ? Red : Clear; });
        Image image{FullScreen("sprite")};

        Draw(image);

        EXPECT_EQ(UIPixel(1, 8), Red);
        EXPECT_EQ(UIPixel(14, 8)[3], 0);
    }

    TEST_F(ImageDrawTests, OpaqueTexturesCoverTheBackground) {
        AddTexture("hero", 1, 1, [](int, int) { return Red; });
        Image image{FullScreen("hero", GreenColor)};

        Draw(image);

        EXPECT_EQ(UIPixel(8, 8), Red);
    }

    TEST_F(ImageDrawTests, OpacityFadesTheWholeImage) {
        AddTexture("hero", 1, 1, [](int, int) { return Red; });
        Image image{FullScreen("hero")};
        image.SetOpacity(0.5f);

        Draw(image);

        const Pixel pixel = UIPixel(8, 8);
        EXPECT_NEAR(pixel[0], 128, 2);
        EXPECT_EQ(pixel[1], 0);
    }

    TEST_F(ImageDrawTests, ABorderRadiusRoundsTheImageCorners) {
        AddTexture("hero", 1, 1, [](int, int) { return Red; });
        Image image{FullScreen("hero")};
        image.SetBorderRadius(8.0f);

        Draw(image);

        EXPECT_EQ(UIPixel(0, 0)[3], 0);
        EXPECT_EQ(UIPixel(8, 8), Red);
    }

    TEST_F(ImageDrawTests, WithoutATextureCacheTheBackgroundIsDrawn) {
        drawer->GetUIDrawList().textures = nullptr;
        Image image{FullScreen("hero", GreenColor)};

        Draw(image);

        EXPECT_EQ(UIPixel(8, 8), Green);
    }

    // A textured command must not leave its texture bound for the plain rectangles drawn after it.
    TEST_F(ImageDrawTests, PlainRectanglesAfterAnImageAreNotTextured) {
        AddTexture("hero", 1, 1, [](int, int) { return Red; });
        Image image{FullScreen("hero")};
        image.GetDrawCommand(drawer->GetUIDrawList(), 1);
        Image background{FullScreen("hero", GreenColor)};
        drawer->GetUIDrawList().textures = nullptr;
        background.SetDrawOrder(200);
        background.GetDrawCommand(drawer->GetUIDrawList(), 1);

        const auto size = static_cast<float>(TargetSize);
        drawer->DrawUI(MatrixFactory::CreateOrtho(0.0f, size, size, 0.0f, -1.0f, 1.0f));

        EXPECT_EQ(UIPixel(8, 8), Green);
    }

    TEST_F(ImageDrawTests, TheDrawOrderStillWinsOverTheTexture) {
        AddTexture("red", 1, 1, [](int, int) { return Red; });
        AddTexture("blue", 1, 1, [](int, int) { return Blue; });
        const bool redHasTheLowerId = textures->Get("red").GetID() < textures->Get("blue").GetID();
        const std::string lowId = redHasTheLowerId ? "red" : "blue";
        const std::string highId = redHasTheLowerId ? "blue" : "red";
        const Pixel lowColor = redHasTheLowerId ? Red : Blue;

        Image top{FullScreen(lowId)};
        top.SetDrawOrder(20);
        Image bottom{FullScreen(highId)};
        bottom.SetDrawOrder(10);
        top.GetDrawCommand(drawer->GetUIDrawList(), 1);
        bottom.GetDrawCommand(drawer->GetUIDrawList(), 1);

        const auto size = static_cast<float>(TargetSize);
        drawer->DrawUI(MatrixFactory::CreateOrtho(0.0f, size, size, 0.0f, -1.0f, 1.0f));

        EXPECT_EQ(UIPixel(8, 8), lowColor);
    }

    // Many images alternating two textures at the same order: grouped by texture, each still draws its own.
    TEST_F(ImageDrawTests, ImagesSharingADrawOrderEachKeepTheirTexture) {
        AddTexture("red", 1, 1, [](int, int) { return Red; });
        AddTexture("blue", 1, 1, [](int, int) { return Blue; });

        std::vector<std::unique_ptr<Image>> images;
        for (int column = 0; column < 16; ++column) {
            ImageInput input{};
            input.anchor = Vector2f{static_cast<float>(column) + 0.5f, 8.0f};
            input.size = Vector2f{1.0f, 16.0f};
            input.backgroundColor = Transparent;
            input.name = column % 2 == 0 ? "red" : "blue";
            images.push_back(std::make_unique<Image>(input));
            images.back()->GetDrawCommand(drawer->GetUIDrawList(), 1);
        }

        const auto size = static_cast<float>(TargetSize);
        drawer->DrawUI(MatrixFactory::CreateOrtho(0.0f, size, size, 0.0f, -1.0f, 1.0f));

        for (int column = 0; column < 16; ++column) {
            EXPECT_EQ(UIPixel(column, 8), column % 2 == 0 ? Red : Blue) << column;
        }
    }
}
