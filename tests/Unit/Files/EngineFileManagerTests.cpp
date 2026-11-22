// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iterator>
#include <stdexcept>
#include <string_view>
#include <string>
#include <system_error>
#include <vector>
#include <gtest/gtest.h>
#include "Assets/AssetCache.h"
#include "Files/EngineFileManager.h"
#include "Graphics/Resources/Font.h"
#include "Graphics/Resources/Model.h"
#include "Graphics/Resources/Texture.h"
#include "Sound/Resources/AudioClip.h"
#include "SpinoCore/Files/FileManager.h"

namespace {
    namespace fs = std::filesystem;
    using SpinoCore::Assets::AssetCache;
    using SpinoCore::Files::AssetType;
    using SpinoCore::Files::EngineFileManager;
    using SpinoCore::Files::FileManagerInput;
    using SpinoCore::Graphics::Resources::Font;
    using SpinoCore::Graphics::Resources::Model;
    using SpinoCore::Graphics::Resources::Texture;
    using SpinoCore::Sound::Resources::AudioClip;
    using Bytes = std::vector<std::uint8_t>;

    [[nodiscard]] Bytes ReadAll(const fs::path& path) {
        std::ifstream file(path, std::ios::binary);
        return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    }

    // The test binary has an empty Assets folder next to it, created at build time.
    // Each test writes only inside its own subfolder of it, removed when the test ends.
    class EngineFileManagerTests : public testing::Test {
    protected:
        void SetUp() override {
            folder = fs::path("EngineFileManagerTests") / testing::UnitTest::GetInstance()->current_test_info()->name();
            fs::remove_all(fileManager.GetRootPath() / folder);
        }

        void TearDown() override {
            std::error_code ignored;
            fs::remove_all(fileManager.GetRootPath() / folder, ignored);
        }

        [[nodiscard]] fs::path OnDisk(const fs::path& relative) const { return fileManager.GetRootPath() / relative; }

        void ProcessUntilReady() {
            for (int frame = 0; frame < 10'000 && !fileManager.IsReady(); ++frame) fileManager.ProcessRequests();
        }

        AssetCache<Texture> textureCache{};
        AssetCache<Font> fontCache{};
        AssetCache<AudioClip> soundCache{};
        AssetCache<Model> modelCache{};
        EngineFileManager fileManager{FileManagerInput{.textureCache = textureCache, .fontCache = fontCache, .modelCache = modelCache, .soundCache = soundCache}};
        fs::path folder;
    };

    TEST_F(EngineFileManagerTests, StartsReady) {
        EXPECT_TRUE(fileManager.IsReady());
    }

    TEST_F(EngineFileManagerTests, ProcessingWithNothingToDoKeepsItReady) {
        fileManager.ProcessRequests();

        EXPECT_TRUE(fileManager.IsReady());
    }

    TEST_F(EngineFileManagerTests, RootIsTheAssetsFolderNextToTheExecutable) {
        EXPECT_TRUE(fileManager.GetRootPath().is_absolute());
        EXPECT_EQ(fileManager.GetRootPath().filename(), "Assets");
        EXPECT_TRUE(fs::is_directory(fileManager.GetRootPath()));
    }

    TEST_F(EngineFileManagerTests, BecomesReadyAgainOnceNothingIsPending) {
        fileManager.WriteToSinglePath(folder / "save.bin", Bytes{1});
        EXPECT_FALSE(fileManager.IsReady());

        fileManager.ProcessRequests();

        EXPECT_TRUE(fileManager.IsReady());
    }

    TEST_F(EngineFileManagerTests, ProcessingWithAnEmptyQueueDoesNotSpendTheFrameBudget) {
        fileManager.WriteToSinglePath(folder / "save.bin", Bytes{1});
        fileManager.ProcessRequests();

        const auto start = std::chrono::steady_clock::now();
        fileManager.ProcessRequests();
        const auto elapsed = std::chrono::steady_clock::now() - start;

        EXPECT_LT(elapsed, std::chrono::milliseconds{2});
    }

    TEST_F(EngineFileManagerTests, WritesWaitForTheNextProcessing) {
        const fs::path path = folder / "save.bin";

        fileManager.WriteToSinglePath(path, Bytes{1, 2, 3});

        EXPECT_FALSE(fileManager.IsReady());
        EXPECT_FALSE(fs::exists(OnDisk(path)));

        fileManager.ProcessRequests();

        EXPECT_TRUE(fileManager.IsReady());
        EXPECT_EQ(ReadAll(OnDisk(path)), (Bytes{1, 2, 3}));
    }

    TEST_F(EngineFileManagerTests, WriteKeepsItsOwnCopyOfTheBytes) {
        const fs::path path = folder / "save.bin";
        {
            Bytes data{9, 8, 7};
            fileManager.WriteToSinglePath(path, data);
            data.assign({0, 0, 0});
        }

        fileManager.ProcessRequests();

        EXPECT_EQ(ReadAll(OnDisk(path)), (Bytes{9, 8, 7}));
    }

    TEST_F(EngineFileManagerTests, RequestsRunInTheOrderTheyWereMade) {
        const fs::path path = folder / "save.bin";
        fileManager.WriteToSinglePath(path, Bytes{1});
        fileManager.WriteToSinglePath(path, Bytes{2});

        ProcessUntilReady();

        EXPECT_EQ(ReadAll(OnDisk(path)), (Bytes{2}));
    }

    TEST_F(EngineFileManagerTests, LongQueuesFinishOverSeveralFrames) {
        constexpr int count = 200;
        for (int i = 0; i < count; ++i) {
            fileManager.WriteToSinglePath(folder / ("file" + std::to_string(i)), Bytes(4096, static_cast<std::uint8_t>(i)));
        }

        ProcessUntilReady();

        ASSERT_TRUE(fileManager.IsReady());
        for (int i = 0; i < count; ++i) {
            EXPECT_TRUE(fs::is_regular_file(OnDisk(folder / ("file" + std::to_string(i))))) << i;
        }
    }

    TEST_F(EngineFileManagerTests, FileExistsIsRelativeToTheAssetsFolder) {
        const fs::path path = folder / "save.bin";
        EXPECT_FALSE(fileManager.FileExists(path));

        fileManager.WriteToSinglePath(path, Bytes{1});
        fileManager.ProcessRequests();

        EXPECT_TRUE(fileManager.FileExists(path));
        EXPECT_FALSE(fileManager.FileExists(folder));
        EXPECT_FALSE(fileManager.FileExists(OnDisk(path)));
    }

    TEST_F(EngineFileManagerTests, AbsolutePathsAreRejected) {
        const fs::path outside = fs::temp_directory_path() / "SpinoCoreEngineFileManagerOutside.bin";
        fs::remove(outside);

        fileManager.WriteToSinglePath(outside, Bytes{1});
        fileManager.LoadFromSinglePath(AssetType::Texture, "outside", outside);

        EXPECT_TRUE(fileManager.IsReady());
        fileManager.ProcessRequests();
        EXPECT_FALSE(fs::exists(outside));
        EXPECT_FALSE(textureCache.Contains("outside"));
    }

    // ---------------------------------------------------------------- manifests
    // Loads are only queued here, never processed: these tests have no GL context.

    class EngineFileManagerManifestTests : public EngineFileManagerTests {
    protected:
        // Writes the manifest straight to disk under the test's folder and returns its path relative to Assets.
        [[nodiscard]] fs::path Manifest(const std::string_view json) const {
            fs::create_directories(OnDisk(folder));
            const fs::path relative = folder / "manifest.json";
            std::ofstream(OnDisk(relative), std::ios::binary) << json;
            return relative;
        }

        void ExpectRejected(const std::string_view json) {
            EXPECT_THROW(fileManager.LoadFromManifest(Manifest(json)), std::runtime_error) << json;
            EXPECT_TRUE(fileManager.IsReady()) << "nothing may be queued from a rejected manifest: " << json;
        }
    };

    TEST_F(EngineFileManagerManifestTests, ValidManifestQueuesItsAssets) {
        fileManager.LoadFromManifest(Manifest(R"({"version": "1.0", "assets": [
            {"name": "hero", "type": "Texture", "path": "Textures/hero.png"},
            {"name": "tree", "type": "Texture", "path": "Textures/tree.png"}
        ]})"));

        EXPECT_FALSE(fileManager.IsReady());
    }

    TEST_F(EngineFileManagerManifestTests, ExtraFieldsAreIgnored) {
        fileManager.LoadFromManifest(Manifest(R"({"version": "1.0", "type": "scene", "assets": [
            {"name": "hero", "type": "Texture", "path": "hero.png", "comment": "main character"}
        ]})"));

        EXPECT_FALSE(fileManager.IsReady());
    }

    TEST_F(EngineFileManagerManifestTests, ManifestWithoutAssetsQueuesNothing) {
        fileManager.LoadFromManifest(Manifest(R"({"version": "1.0", "assets": []})"));

        EXPECT_TRUE(fileManager.IsReady());
    }

    TEST_F(EngineFileManagerManifestTests, MissingManifestThrows) {
        EXPECT_THROW(fileManager.LoadFromManifest(folder / "missing.json"), std::runtime_error);
        EXPECT_TRUE(fileManager.IsReady());
    }

    TEST_F(EngineFileManagerManifestTests, ManifestPathIsRelativeToTheAssetsFolder) {
        const fs::path relative = Manifest(R"({"version": "1.0", "assets": []})");

        EXPECT_NO_THROW(fileManager.LoadFromManifest(relative));
        EXPECT_THROW(fileManager.LoadFromManifest(OnDisk(relative)), std::runtime_error);
    }

    TEST_F(EngineFileManagerManifestTests, InvalidJSONIsRejected) {
        ExpectRejected(R"({"version": "1.0", "assets": [}")");
        ExpectRejected("");
    }

    TEST_F(EngineFileManagerManifestTests, VersionMustBeTheString1_0) {
        ExpectRejected(R"({"assets": []})");
        ExpectRejected(R"({"version": "2.0", "assets": []})");
        ExpectRejected(R"({"version": 1.0, "assets": []})");
    }

    TEST_F(EngineFileManagerManifestTests, AssetsMustBeAnArray) {
        ExpectRejected(R"({"version": "1.0"})");
        ExpectRejected(R"({"version": "1.0", "assets": {"name": "hero"}})");
    }

    TEST_F(EngineFileManagerManifestTests, EachAssetNeedsANameATypeAndAPath) {
        ExpectRejected(R"({"version": "1.0", "assets": ["hero.png"]})");
        ExpectRejected(R"({"version": "1.0", "assets": [{"type": "Texture", "path": "hero.png"}]})");
        ExpectRejected(R"({"version": "1.0", "assets": [{"name": "", "type": "Texture", "path": "hero.png"}]})");
        ExpectRejected(R"({"version": "1.0", "assets": [{"name": "hero", "path": "hero.png"}]})");
        ExpectRejected(R"({"version": "1.0", "assets": [{"name": "hero", "type": "Texture"}]})");
        ExpectRejected(R"({"version": "1.0", "assets": [{"name": "hero", "type": "Texture", "path": 7}]})");
    }

    TEST_F(EngineFileManagerManifestTests, UnknownAssetTypesAreRejected) {
        ExpectRejected(R"({"version": "1.0", "assets": [{"name": "hero", "type": "Video", "path": "hero.mp4"}]})");
        ExpectRejected(R"({"version": "1.0", "assets": [{"name": "hero", "type": "None", "path": "hero.png"}]})");
        ExpectRejected(R"({"version": "1.0", "assets": [{"name": "hero", "type": "texture", "path": "hero.png"}]})");
    }

    TEST_F(EngineFileManagerManifestTests, AssetPathsMustBeRelative) {
        const std::string absolute = (fs::temp_directory_path() / "hero.png").generic_string();
        ExpectRejected(R"({"version": "1.0", "assets": [{"name": "hero", "type": "Texture", "path": ")" + absolute + R"("}]})");
    }

    TEST_F(EngineFileManagerManifestTests, OneBrokenAssetRejectsTheWholeManifest) {
        ExpectRejected(R"({"version": "1.0", "assets": [
            {"name": "hero", "type": "Texture", "path": "hero.png"},
            {"name": "tree", "type": "Texture"}
        ]})");
    }

    TEST(EngineFileManagerConstructionTests, FindsTheAssetsFolderFromAnyWorkingDirectory) {
        const auto previous = fs::current_path();
        const auto elsewhere = fs::temp_directory_path() / "SpinoCoreNoAssets";
        fs::create_directories(elsewhere);
        fs::current_path(elsewhere);

        AssetCache<Texture> textureCache{};
        AssetCache<Font> fontCache{};
        AssetCache<AudioClip> soundCache{};
        AssetCache<Model> modelCache{};
        EXPECT_NO_THROW((EngineFileManager{FileManagerInput{.textureCache = textureCache, .fontCache = fontCache, .modelCache = modelCache, .soundCache = soundCache}}));

        fs::current_path(previous);
        fs::remove_all(elsewhere);
    }
}
