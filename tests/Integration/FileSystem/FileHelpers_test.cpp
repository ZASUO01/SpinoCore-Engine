// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <span>
#include "FileSystem/Helpers//FileHelpers.h"

using namespace SpinoCore::FileSystem::Helpers;

namespace {
    class FileHelpersIntegrationTest : public testing::Test {
    protected:
        std::filesystem::path testDir;

        void SetUp() override {
            testDir = std::filesystem::temp_directory_path() / "SpinoCore_FileTests";
            std::filesystem::create_directories(testDir);
        }

        void TearDown() override {
            if (std::filesystem::exists(testDir)) {
                std::filesystem::remove_all(testDir);
            }
        }

        static void CreateTestFile(const std::filesystem::path& filepath, const std::string_view content) {
            std::ofstream out(filepath, std::ios::binary | std::ios::trunc);
            out.write(content.data(), static_cast<std::streamsize>(content.size()));
        }

        static std::string ReadTestFile(const std::filesystem::path& filepath) {
            std::ifstream in(filepath, std::ios::binary);
            return {(std::istreambuf_iterator(in)), std::istreambuf_iterator<char>()};
        }
    };
}



TEST_F(FileHelpersIntegrationTest, ShouldReadAllBytesFromExistingFile) {
    const auto path = testDir / "read_all.txt";
    CreateTestFile(path, "this is a test text");

    const auto data = ReadFile(path);

    ASSERT_TRUE(data.has_value());
    const std::string result(data->begin(), data->end());
    EXPECT_EQ(result, "this is a test text");
}

TEST_F(FileHelpersIntegrationTest, ShouldReturnNulloptForNonExistentFile) {
    const auto path = testDir / "ghost.bin";

    const auto data = ReadFile(path);

    EXPECT_FALSE(data.has_value());
}

TEST_F(FileHelpersIntegrationTest, ShouldReadPartialChunkUsingOffsetAndSize) {
    const auto path = testDir / "test_chunks.pak";

    CreateTestFile(path, "TEXT1|TEXT2|TEXT3");

    const auto data = ReadFile(path, 6, 5);

    ASSERT_TRUE(data.has_value());
    const std::string result(data->begin(), data->end());
    EXPECT_EQ(result, "TEXT2");
}

TEST_F(FileHelpersIntegrationTest, ShouldClampSizeIfRequestedMoreThanAvailable) {
    const auto path = testDir / "clamp_test.bin";
    CreateTestFile(path, "SHORT");

    const auto data = ReadFile(path, 2, 100);

    ASSERT_TRUE(data.has_value());
    const std::string result(data->begin(), data->end());

    EXPECT_EQ(result, "ORT");
    EXPECT_EQ(data->size(), 3);
}

TEST_F(FileHelpersIntegrationTest, ShouldReturnEmptyVectorIfOffsetIsOutOfBounds) {
    const auto path = testDir / "bounds.bin";
    CreateTestFile(path, "DATA");

    const auto data = ReadFile(path, 10);

    ASSERT_TRUE(data.has_value());
    EXPECT_TRUE(data->empty());
}

TEST_F(FileHelpersIntegrationTest, ShouldCreateNewFileAndWriteAllBytes) {
    const auto path = testDir / "write.bin";
    const bool success = WriteToFile(path, {'C', 'H', 'U', 'N', 'K'});

    ASSERT_TRUE(success);
    EXPECT_TRUE(std::filesystem::exists(path));
    EXPECT_EQ(ReadTestFile(path), "CHUNK");
}

TEST_F(FileHelpersIntegrationTest, ShouldOverwriteExistingFileIfNoOffsetProvided) {
    const auto path = testDir / "overwrite.bin";
    CreateTestFile(path, "OLD_MASSIVE_DATA_FILE");

    const bool success = WriteToFile(path, {'N', 'E', 'W'});

    ASSERT_TRUE(success);

    EXPECT_EQ(ReadTestFile(path), "NEW");
    EXPECT_EQ(std::filesystem::file_size(path), 3);
}

TEST_F(FileHelpersIntegrationTest, ShouldPatchExistingFileAtSpecificOffset) {
    const auto path = testDir / "patch.bin";
    CreateTestFile(path, "HELLO_WORLD");

    const bool success = WriteToFile(path, {'S', 'P', 'I', 'N', 'O'},6);

    ASSERT_TRUE(success);

    EXPECT_EQ(ReadTestFile(path), "HELLO_SPINO");
    EXPECT_EQ(std::filesystem::file_size(path), 11);
}