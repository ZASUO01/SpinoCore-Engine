// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <system_error>
#include "FileSystem/Mount/FolderMount.h"

#include <thread>

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

TEST_F(FolderMountIntegrationTest, ShouldHandleConcurrentReadsSafely) {
    const auto mount = FolderMount::Create(testDir, true);
    ASSERT_NE(mount, nullptr);

    CreateRawFile(testDir / "shared_asset.txt", "CONCURRENT_DATA");

    constexpr int NUM_THREADS = 10;
    constexpr int READS_PER_THREAD = 100;
    std::atomic successfulReads{0};

    std::vector<std::jthread> readers;
    for (int i = 0; i < NUM_THREADS; ++i) {
        readers.emplace_back([&] {
            for (int j = 0; j < READS_PER_THREAD; ++j) {
                if (auto data = mount->Read("shared_asset.txt"); data && std::string(data->begin(), data->end()) == "CONCURRENT_DATA") {
                    ++successfulReads;
                }
            }
        });
    }
    readers.clear();

    EXPECT_EQ(successfulReads.load(), NUM_THREADS * READS_PER_THREAD);
}

TEST_F(FolderMountIntegrationTest, ShouldHandleConcurrentWritesSafely) {
    const auto mount = FolderMount::Create(testDir, false);
    ASSERT_NE(mount, nullptr);

    constexpr int NUM_THREADS = 10;
    std::atomic successfulWrites{0};

    std::vector<std::jthread> writers;
    for (int i = 0; i < NUM_THREADS; ++i) {
        writers.emplace_back([&, i] {
            const std::string filename = "thread_output_" + std::to_string(i) + ".bin";
            std::string content = "THREAD_DATA_" + std::to_string(i);

            if (const std::vector<uint8_t> data(content.begin(), content.end()); mount->Write(filename, data)) {
                ++successfulWrites;
            }
        });
    }
    writers.clear();

    EXPECT_EQ(successfulWrites.load(), NUM_THREADS);

    for (int i = 0; i < NUM_THREADS; ++i) {
        std::string filename = "thread_output_" + std::to_string(i) + ".bin";
        auto data = mount->Read(filename);

        ASSERT_TRUE(data.has_value());
        EXPECT_EQ(std::string(data->begin(), data->end()), "THREAD_DATA_" + std::to_string(i));
    }
}