// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <filesystem>
#include <fstream>
#include <ios>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>
#include <gtest/gtest.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>
#include "Assets/AssetCache.h"
#include "Files/EngineFileManager.h"
#include "../Graphics/GLTestContext.h"
#include "Graphics/Resources/Font.h"
#include "Graphics/Resources/Texture.h"
#include "SpinoCore/Files/FileManager.h"

namespace {
    namespace fs = std::filesystem;
    using SpinoCore::Assets::AssetCache;
    using SpinoCore::Files::AssetType;
    using SpinoCore::Files::EngineFileManager;
    using SpinoCore::Files::FileManagerInput;
    using SpinoCore::Graphics::Resources::Font;
    using SpinoCore::Graphics::Resources::Texture;

    // Loads real images from the Assets folder next to the test binary into a cache, with a live GL context.
    class EngineFileManagerTextureTests : public SpinoCore::Tests::GLTestContext {
    protected:
        void SetUp() override {
            GLTestContext::SetUp();
            if (HasFatalFailure()) return;
            textureCache.emplace(std::make_unique<const Texture>());
            fileManager.emplace(FileManagerInput{.textureCache = *textureCache, .fontCache = fontCache});
            folder = fs::path("EngineFileManagerTextureTests") / testing::UnitTest::GetInstance()->current_test_info()->name();
            fs::create_directories(fileManager->GetRootPath() / folder);
        }

        void TearDown() override {
            if (fileManager) {
                std::error_code ignored;
                fs::remove_all(fileManager->GetRootPath() / folder, ignored);
            }
            // Textures go before the GL context the fixture owns.
            fileManager.reset();
            textureCache.reset();
            GLTestContext::TearDown();
        }

        // Saves a width x height BMP under the test's folder and returns its path relative to Assets.
        [[nodiscard]] fs::path WriteImage(const std::string& name, const int width, const int height) const {
            SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
            EXPECT_NE(surface, nullptr) << SDL_GetError();
            const fs::path relative = folder / name;
            EXPECT_TRUE(SDL_SaveBMP(surface, (fileManager->GetRootPath() / relative).string().c_str())) << SDL_GetError();
            SDL_DestroySurface(surface);
            return relative;
        }

        // Requests past the frame budget wait for the next frame, so a slow machine may need more than one.
        void ProcessUntilReady() {
            for (int frame = 0; frame < 1'000 && !fileManager->IsReady(); ++frame) fileManager->ProcessRequests();
        }

        std::optional<AssetCache<Texture>> textureCache{};
        AssetCache<Font> fontCache{};
        std::optional<EngineFileManager> fileManager{};
        fs::path folder;
    };

    TEST_F(EngineFileManagerTextureTests, LoadedTexturesAreCachedUnderTheirId) {
        const fs::path image = WriteImage("hero.bmp", 3, 2);

        fileManager->LoadFromSinglePath(AssetType::Texture, "hero", image);
        EXPECT_FALSE(textureCache->Contains("hero"));

        fileManager->ProcessRequests();

        ASSERT_TRUE(textureCache->Contains("hero"));
        EXPECT_EQ(textureCache->Get("hero").GetSize().width, 3);
        EXPECT_EQ(textureCache->Get("hero").GetSize().height, 2);
        EXPECT_TRUE(fileManager->IsReady());
    }

    TEST_F(EngineFileManagerTextureTests, MissingFilesFallBackToTheDefaultTexture) {
        fileManager->LoadFromSinglePath(AssetType::Texture, "ghost", folder / "missing.png");

        fileManager->ProcessRequests();

        EXPECT_TRUE(fileManager->IsReady());
        EXPECT_FALSE(textureCache->Contains("ghost"));
        EXPECT_EQ(&textureCache->GetOrDefault("ghost"), &textureCache->GetDefault());
    }

    TEST_F(EngineFileManagerTextureTests, UndecodableFilesFallBackToTheDefaultTexture) {
        const fs::path fake = folder / "fake.png";
        fileManager->WriteToSinglePath(fake, {'n', 'o', 't', ' ', 'p', 'n', 'g'});
        fileManager->LoadFromSinglePath(AssetType::Texture, "fake", fake);

        ProcessUntilReady();

        EXPECT_TRUE(fileManager->IsReady());
        EXPECT_FALSE(textureCache->Contains("fake"));
        EXPECT_EQ(&textureCache->GetOrDefault("fake"), &textureCache->GetDefault());
    }

    TEST_F(EngineFileManagerTextureTests, ADuplicatedIdKeepsTheFirstTexture) {
        const fs::path first = WriteImage("first.bmp", 4, 4);
        const fs::path second = WriteImage("second.bmp", 8, 8);

        fileManager->LoadFromSinglePath(AssetType::Texture, "hero", first);
        fileManager->LoadFromSinglePath(AssetType::Texture, "hero", second);
        ProcessUntilReady();

        ASSERT_TRUE(textureCache->Contains("hero"));
        EXPECT_EQ(textureCache->Get("hero").GetSize().width, 4);
        EXPECT_EQ(textureCache->Size(), 1u);
    }

    TEST_F(EngineFileManagerTextureTests, OneFailedLoadDoesNotStopTheOthers) {
        const fs::path good = WriteImage("good.bmp", 2, 2);

        fileManager->LoadFromSinglePath(AssetType::Texture, "bad", folder / "missing.png");
        fileManager->LoadFromSinglePath(AssetType::Texture, "good", good);
        ProcessUntilReady();

        EXPECT_FALSE(textureCache->Contains("bad"));
        EXPECT_TRUE(textureCache->Contains("good"));
    }

    TEST_F(EngineFileManagerTextureTests, ManifestLoadsEveryTextureItLists) {
        const fs::path hero = WriteImage("hero.bmp", 3, 2);
        const fs::path tree = WriteImage("tree.bmp", 5, 4);
        const fs::path manifest = folder / "manifest.json";
        std::ofstream(fileManager->GetRootPath() / manifest, std::ios::binary)
            << R"({"version": "1.0", "assets": [)"
            << R"({"name": "hero", "type": "Texture", "path": ")" << hero.generic_string() << R"("},)"
            << R"({"name": "tree", "type": "Texture", "path": ")" << tree.generic_string() << R"("}]})";

        fileManager->LoadFromManifest(manifest);
        ProcessUntilReady();

        ASSERT_TRUE(fileManager->IsReady());
        ASSERT_TRUE(textureCache->Contains("hero"));
        ASSERT_TRUE(textureCache->Contains("tree"));
        EXPECT_EQ(textureCache->Get("hero").GetSize().width, 3);
        EXPECT_EQ(textureCache->Get("tree").GetSize().height, 4);
    }

    TEST_F(EngineFileManagerTextureTests, AMissingImageInTheManifestFallsBackWithoutStoppingTheOthers) {
        const fs::path hero = WriteImage("hero.bmp", 2, 2);
        const fs::path manifest = folder / "manifest.json";
        std::ofstream(fileManager->GetRootPath() / manifest, std::ios::binary)
            << R"({"version": "1.0", "assets": [)"
            << R"({"name": "ghost", "type": "Texture", "path": "missing.png"},)"
            << R"({"name": "hero", "type": "Texture", "path": ")" << hero.generic_string() << R"("}]})";

        fileManager->LoadFromManifest(manifest);
        ProcessUntilReady();

        EXPECT_TRUE(fileManager->IsReady());
        EXPECT_TRUE(textureCache->Contains("hero"));
        EXPECT_EQ(&textureCache->GetOrDefault("ghost"), &textureCache->GetDefault());
    }

    TEST_F(EngineFileManagerTextureTests, TexturesThatFailToLoadAreReportedByTheirId) {
        const fs::path good = WriteImage("good.bmp", 2, 2);

        fileManager->LoadFromSinglePath(AssetType::Texture, "bad", folder / "missing.png");
        fileManager->LoadFromSinglePath(AssetType::Texture, "good", good);
        ProcessUntilReady();

        EXPECT_EQ(fileManager->GetFailedLoads(), (std::vector<std::string>{"bad"}));
    }

    TEST_F(EngineFileManagerTextureTests, ANewBatchOfLoadsForgetsThePreviousFailures) {
        const fs::path good = WriteImage("good.bmp", 2, 2);
        fileManager->LoadFromSinglePath(AssetType::Texture, "bad", folder / "missing.png");
        ProcessUntilReady();

        fileManager->LoadFromSinglePath(AssetType::Texture, "good", good);
        ProcessUntilReady();

        EXPECT_TRUE(fileManager->GetFailedLoads().empty());
    }
}
