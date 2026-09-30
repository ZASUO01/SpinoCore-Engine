// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include "FileSystem/Core/VirtualFileSystem.h"
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
    auto vfs = VirtualFileSystem::Create();

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