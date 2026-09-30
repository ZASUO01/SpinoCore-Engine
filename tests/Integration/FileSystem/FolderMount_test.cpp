// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <system_error>
#include "FileSystem/Mount/FolderMount.h"

using namespace SpinoCore::FileSystem;
using namespace SpinoCore::FileSystem::Mount;

namespace {
    class FolderMountIntegrationTest : public testing::Test {
    protected:
        std::filesystem::path testDir;

        void SetUp() override {
            testDir = std::filesystem::temp_directory_path() / "SpinoCore_FolderMountTests";
            std::error_code ec;
            std::filesystem::create_directories(testDir, ec);
            ASSERT_FALSE(ec);
        }

        void TearDown() override {
            if (std::error_code ec; std::filesystem::exists(testDir, ec)) {
                std::filesystem::remove_all(testDir, ec);
            }
        }

        static void CreateRawFile(const std::filesystem::path& filepath, const std::string_view content) {
            std::ofstream out(filepath, std::ios::binary | std::ios::trunc);
            out.write(content.data(), static_cast<std::streamsize>(content.size()));
        }
    };
}

TEST_F(FolderMountIntegrationTest, ShouldFailToMountIfDirectoryDoesNotExist) {
    const auto ghostPath = testDir / "ghost_folder";
    const auto mount = FolderMount::Create(ghostPath, true);

    EXPECT_EQ(mount, nullptr);
}

TEST_F(FolderMountIntegrationTest, ShouldFailToMountIfPathIsAFileNotADirectory) {
    const auto filePath = testDir / "file_not_dir.txt";
    CreateRawFile(filePath, "DUMMY");

    const auto mount = FolderMount::Create(filePath, true);

    EXPECT_EQ(mount, nullptr);
}

TEST_F(FolderMountIntegrationTest, ShouldMountSuccessfullyIfDirectoryExists) {
    const auto mount = FolderMount::Create(testDir, true);

    ASSERT_NE(mount, nullptr);
    EXPECT_TRUE(mount->IsReadOnly());
}

TEST_F(FolderMountIntegrationTest, ShouldPreventPathTraversalAttacks) {
    const auto mount = FolderMount::Create(testDir, false);
    ASSERT_NE(mount, nullptr);

    EXPECT_FALSE(mount->Exists("../secret.txt"));
    EXPECT_FALSE(mount->Read("subfolder/../../secret.txt").has_value());
    EXPECT_FALSE(mount->Write("../hacked.txt", {'H','A','C','K'}));
}

TEST_F(FolderMountIntegrationTest, ShouldPreventAbsolutePaths) {
    const auto mount = FolderMount::Create(testDir, false);
    ASSERT_NE(mount, nullptr);

    #ifdef _WIN32
        EXPECT_FALSE(mount->Exists("C:\\Windows\\System32\\cmd.exe"));
    #else
        EXPECT_FALSE(mount->Exists("/etc/passwd"));
    #endif
}

TEST_F(FolderMountIntegrationTest, ShouldReturnTrueForExistingFile) {
    const auto mount = FolderMount::Create(testDir, true);
    ASSERT_NE(mount, nullptr);

    CreateRawFile(testDir / "model.obj", "VERTEX_DATA");

    EXPECT_TRUE(mount->Exists("model.obj"));
    EXPECT_FALSE(mount->Exists("missing_model.obj"));
}

TEST_F(FolderMountIntegrationTest, ShouldFailGracefullyWithEmptyPaths) {
    const auto mount = FolderMount::Create(testDir, false);
    ASSERT_NE(mount, nullptr);

    EXPECT_FALSE(mount->Exists(""));
    EXPECT_FALSE(mount->Read("").has_value());
    EXPECT_FALSE(mount->Write("", {'D', 'A', 'T', 'A'}));
}

TEST_F(FolderMountIntegrationTest, ShouldFailToReadOrWriteIfTargetIsADirectory) {
    const auto mount = FolderMount::Create(testDir, false);
    ASSERT_NE(mount, nullptr);

    std::error_code ec;
    std::filesystem::create_directories(testDir / "Assets", ec);

    EXPECT_FALSE(mount->Exists("Assets"));
    EXPECT_FALSE(mount->Read("Assets").has_value());
    EXPECT_FALSE(mount->Write("Assets", {0x01}));
}

TEST_F(FolderMountIntegrationTest, ShouldReadExistingFileCorrectly) {
    const auto mount = FolderMount::Create(testDir, true);
    ASSERT_NE(mount, nullptr);

    CreateRawFile(testDir / "config.ini", "FULLSCREEN=1");

    const auto data = mount->Read("config.ini");
    ASSERT_TRUE(data.has_value());

    const std::string result(data->begin(), data->end());
    EXPECT_EQ(result, "FULLSCREEN=1");
}

TEST_F(FolderMountIntegrationTest, ShouldFailToWriteIfMountIsReadOnly) {
    const auto mount = FolderMount::Create(testDir, true);
    ASSERT_NE(mount, nullptr);

    EXPECT_FALSE(mount->Write("test.bin", {'D', 'A', 'T', 'A'}));
    EXPECT_FALSE(std::filesystem::exists(testDir / "test.bin"));
}

TEST_F(FolderMountIntegrationTest, ShouldWriteSuccessfullyToWritableMount) {
    const auto mount = FolderMount::Create(testDir, false);
    ASSERT_NE(mount, nullptr);

    EXPECT_TRUE(mount->Write("save.dat", {'S', 'A', 'V', 'E'}));

    const auto targetFile = testDir / "save.dat";
    EXPECT_TRUE(std::filesystem::exists(targetFile));
}

TEST_F(FolderMountIntegrationTest, ShouldCreateNestedDirectoriesOnWrite) {
    const auto mount = FolderMount::Create(testDir, false);
    ASSERT_NE(mount, nullptr);

    EXPECT_TRUE(mount->Write("deep/nested/folder/data.bin", {0x01, 0x02}));

    const auto expectedPath = testDir / "deep" / "nested" / "folder" / "data.bin";
    EXPECT_TRUE(std::filesystem::exists(expectedPath));
}

TEST_F(FolderMountIntegrationTest, ShouldOverwriteExistingFileCorrectly) {
    const auto mount = FolderMount::Create(testDir, false);
    ASSERT_NE(mount, nullptr);

    EXPECT_TRUE(mount->Write("config.ini", {'O', 'L', 'D'}));
    EXPECT_TRUE(mount->Write("config.ini", {'N', 'E', 'W'}));

    const auto data = mount->Read("config.ini");
    ASSERT_TRUE(data.has_value());

    const std::string result(data->begin(), data->end());
    EXPECT_EQ(result, "NEW");
}