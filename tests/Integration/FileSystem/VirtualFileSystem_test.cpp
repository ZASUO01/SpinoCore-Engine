// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include "FileSystem/Core/VirtualFileSystem.h"

#include <thread>

#include "FileSystem/Mount/FolderMount.h"

using namespace SpinoCore::FileSystem;
using namespace SpinoCore::FileSystem::Core;
using namespace SpinoCore::FileSystem::Mount;

namespace {
    class VirtualFileSystemIntegrationTest : public testing::Test {
    protected:
        std::filesystem::path testRoot;
        std::filesystem::path baseDir;
        std::filesystem::path modDir;
        std::filesystem::path saveDir;

        void SetUp() override {
            testRoot = std::filesystem::temp_directory_path() / "SpinoCore_VFS_Integration";
            baseDir = testRoot / "base_game";
            modDir = testRoot / "mods";
            saveDir = testRoot / "saves";

            std::error_code ec;
            std::filesystem::create_directories(baseDir, ec);
            ASSERT_FALSE(ec);

            std::filesystem::create_directories(modDir, ec);
            ASSERT_FALSE(ec);

            std::filesystem::create_directories(saveDir, ec);
            ASSERT_FALSE(ec);
        }

        void TearDown() override {
            if (std::error_code ec; std::filesystem::exists(testRoot, ec)) {
                std::filesystem::remove_all(testRoot, ec);
            }
        }

        static void CreateRawFile(const std::filesystem::path& filepath, const std::string_view content) {
            std::ofstream out(filepath, std::ios::binary | std::ios::trunc);
            out.write(content.data(), static_cast<std::streamsize>(content.size()));
        }
    };
}


TEST_F(VirtualFileSystemIntegrationTest, ShouldRouteReadAndWriteToCorrectMounts) {
    const auto vfs = VirtualFileSystem::Create();

    ASSERT_TRUE(vfs->Mount<FolderMount>("res", baseDir, true));
    ASSERT_TRUE(vfs->Mount<FolderMount>("user", saveDir, false));

    CreateRawFile(baseDir / "config.ini", "VSYNC=1");

    const auto readData = vfs->Read("res://config.ini");
    ASSERT_TRUE(readData.has_value());
    const std::string result(readData->begin(), readData->end());
    EXPECT_EQ(result, "VSYNC=1");

    EXPECT_FALSE(vfs->Read("res://missing_texture.png").has_value());

    EXPECT_TRUE(vfs->Write("user://slot1.dat", {'S', 'A', 'V', 'E'}));
    EXPECT_TRUE(std::filesystem::exists(saveDir / "slot1.dat"));
    EXPECT_FALSE(std::filesystem::exists(baseDir / "slot1.dat"));
}

TEST_F(VirtualFileSystemIntegrationTest, ShouldPrioritizeLastMountedFolderForReads) {
    const auto vfs = VirtualFileSystem::Create();

    ASSERT_TRUE(vfs->Mount<FolderMount>("data", baseDir, true));
    ASSERT_TRUE(vfs->Mount<FolderMount>("data", modDir, true));

    CreateRawFile(baseDir / "base_only.txt", "BASE_EXCLUSIVE");
    CreateRawFile(baseDir / "overwritten.txt", "ORIGINAL_TEXTURE");
    CreateRawFile(modDir / "overwritten.txt", "MODDED_TEXTURE");

    auto data1 = vfs->Read("data://base_only.txt");
    ASSERT_TRUE(data1.has_value());
    EXPECT_EQ(std::string(data1->begin(), data1->end()), "BASE_EXCLUSIVE");

    auto data2 = vfs->Read("data://overwritten.txt");
    ASSERT_TRUE(data2.has_value());
    EXPECT_EQ(std::string(data2->begin(), data2->end()), "MODDED_TEXTURE");
}

TEST_F(VirtualFileSystemIntegrationTest, ShouldWriteToFirstWritableMountInStack) {
    const auto vfs = VirtualFileSystem::Create();

    ASSERT_TRUE(vfs->Mount<FolderMount>("data", baseDir, true));
    ASSERT_TRUE(vfs->Mount<FolderMount>("data", saveDir, false));

    EXPECT_TRUE(vfs->Write("data://custom_config.ini",  {'N', 'E', 'W'}));

    EXPECT_FALSE(std::filesystem::exists(baseDir / "custom_config.ini"));
    EXPECT_TRUE(std::filesystem::exists(saveDir / "custom_config.ini"));
}

TEST_F(VirtualFileSystemIntegrationTest, ShouldFailToWriteIfAllMountsAreReadOnly) {
    const auto vfs = VirtualFileSystem::Create();

    ASSERT_TRUE(vfs->Mount<FolderMount>("res", baseDir, true));
    ASSERT_TRUE(vfs->Mount<FolderMount>("res", modDir, true));

    EXPECT_FALSE(vfs->Write("res://banned_write.bin", {0xFF}));
    EXPECT_FALSE(std::filesystem::exists(baseDir / "banned_write.bin"));
    EXPECT_FALSE(std::filesystem::exists(modDir / "banned_write.bin"));
}

TEST_F(VirtualFileSystemIntegrationTest, ShouldRouteConcurrentReadsWithoutBlocking) {
    const auto vfs = VirtualFileSystem::Create();

    ASSERT_TRUE(vfs->Mount<FolderMount>("res", baseDir, true));
    ASSERT_TRUE(vfs->Mount<FolderMount>("mod", modDir, true));

    CreateRawFile(baseDir / "tex1.png", "TEXTURE_1");
    CreateRawFile(modDir / "model1.obj", "MODEL_1");

    constexpr int NUM_THREADS = 10;
    std::atomic successCount{0};

    std::vector<std::jthread> readers;
    for (int i = 0; i < NUM_THREADS; ++i) {
        readers.emplace_back([&, i] {
            if (i % 2 == 0) {
                for (int k = 0; k < 50; ++k) {
                    if (auto data = vfs->Read("res://tex1.png")) ++successCount;
                }
            } else {
                for (int k = 0; k < 50; ++k) {
                    if (auto data = vfs->Read("mod://model1.obj")) ++successCount;
                }
            }
        });
    }
    readers.clear();

    EXPECT_EQ(successCount.load(), NUM_THREADS * 50);
}

TEST_F(VirtualFileSystemIntegrationTest, ShouldAllowConcurrentReadAndWriteOnDifferentProtocols) {
    const auto vfs = VirtualFileSystem::Create();

    ASSERT_TRUE(vfs->Mount<FolderMount>("res", baseDir, true));
    ASSERT_TRUE(vfs->Mount<FolderMount>("user", saveDir, false));

    CreateRawFile(baseDir / "music.ogg", "AUDIO_DATA");

    std::atomic writeSuccess{false};
    std::atomic readSuccess{0};

    std::jthread writer([&] {
        writeSuccess = vfs->Write("user://autosave.dat", {'S', 'A', 'V', 'E'});
    });

    std::vector<std::jthread> readers;
    for (int i = 0; i < 4; ++i) {
        readers.emplace_back([&] {
            for (int k = 0; k < 20; ++k) {
                if (auto data = vfs->Read("res://music.ogg")) ++readSuccess;
            }
        });
    }

    writer.join();
    readers.clear();

    EXPECT_TRUE(writeSuccess.load());
    EXPECT_EQ(readSuccess.load(), 4 * 20);
    EXPECT_TRUE(std::filesystem::exists(saveDir / "autosave.dat"));
}

TEST_F(VirtualFileSystemIntegrationTest, ShouldSafelyMountNewDrivesWhileReading) {
    const auto vfs = VirtualFileSystem::Create();
    ASSERT_TRUE(vfs->Mount<FolderMount>("sys", baseDir, true));

    CreateRawFile(baseDir / "core.bin", "CORE");

    std::atomic isReading{true};
    std::atomic totalReads{0};

    std::jthread reader([&] {
        while (isReading.load()) {
            if (vfs->Read("sys://core.bin")) {
                ++totalReads;
            }
        }
    });

    std::jthread mounter([&] {
        std::error_code ec;
        auto dlcDir = testRoot / "dlc_1";
        std::filesystem::create_directories(dlcDir, ec);
        CreateRawFile(dlcDir / "dlc_asset.bin", "DLC_DATA");

        EXPECT_TRUE(vfs->Mount<FolderMount>("dlc", dlcDir, true));

        EXPECT_TRUE(vfs->Exists("dlc://dlc_asset.bin"));
        isReading = false;
    });

    mounter.join();
    reader.join();

    EXPECT_GT(totalReads.load(), 0);
}
