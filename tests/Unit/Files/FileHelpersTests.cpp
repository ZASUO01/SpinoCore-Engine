// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <random>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>
#include <gtest/gtest.h>
#include "Files/FileHelpers.h"

namespace {
    namespace fs = std::filesystem;
    using namespace SpinoCore::Files;

    // A fresh folder under the system temp directory, removed when the test ends.
    class TempDirectory final {
    public:
        TempDirectory() {
            std::random_device rd;
            mPath = fs::temp_directory_path() / ("spinocore-files-" + std::to_string(rd()) + std::to_string(rd()));
            fs::create_directories(mPath);
        }
        ~TempDirectory() {
            std::error_code ignored;
            fs::permissions(mPath, fs::perms::owner_all, fs::perm_options::add, ignored);
            fs::remove_all(mPath, ignored);
        }
        TempDirectory(const TempDirectory&) = delete;
        TempDirectory& operator=(const TempDirectory&) = delete;

        [[nodiscard]] const fs::path& Path() const { return mPath; }

    private:
        fs::path mPath;
    };

    void WriteRaw(const fs::path& path, const std::string_view bytes) {
        std::ofstream out(path, std::ios::binary);
        out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    }

    [[nodiscard]] std::vector<std::uint8_t> Bytes(const std::string_view text) {
        return {text.begin(), text.end()};
    }

    // Every byte value, plus bytes that text-mode streams would mangle (\r\n, \x1A, \0).
    [[nodiscard]] std::vector<std::uint8_t> AllByteValues() {
        std::vector<std::uint8_t> data;
        for (int value = 0; value < 256; ++value) data.push_back(static_cast<std::uint8_t>(value));
        for (const std::uint8_t value : {0x0D, 0x0A, 0x1A, 0x00, 0x0D, 0x0A}) data.push_back(value);
        return data;
    }

    class FileHelpersTests : public testing::Test {
    protected:
        TempDirectory dir;
    };
}

// ---------------------------------------------------------------- existence

TEST_F(FileHelpersTests, DirectoryExistsOnlyForFolders) {
    WriteRaw(dir.Path() / "file.txt", "x");
    EXPECT_TRUE(DirectoryExists(dir.Path()));
    EXPECT_FALSE(DirectoryExists(dir.Path() / "file.txt"));
    EXPECT_FALSE(DirectoryExists(dir.Path() / "missing"));
    EXPECT_FALSE(DirectoryExists(""));
}

TEST_F(FileHelpersTests, FileExistsOnlyForRegularFiles) {
    WriteRaw(dir.Path() / "file.txt", "x");
    EXPECT_TRUE(FileExists(dir.Path() / "file.txt"));
    EXPECT_FALSE(FileExists(dir.Path()));
    EXPECT_FALSE(FileExists(dir.Path() / "missing.txt"));
    EXPECT_FALSE(FileExists(""));
}

TEST_F(FileHelpersTests, ExistenceChecksNeverThrowOnInvalidPaths) {
    const fs::path impossible = dir.Path() / "file.txt" / "child";
    WriteRaw(dir.Path() / "file.txt", "x");
    EXPECT_NO_THROW({ EXPECT_FALSE(DirectoryExists(impossible)); });
    EXPECT_NO_THROW({ EXPECT_FALSE(FileExists(impossible)); });
}

// ---------------------------------------------------------------- reading

TEST_F(FileHelpersTests, ReadsEveryByteExactly) {
    const auto expected = AllByteValues();
    WriteRaw(dir.Path() / "bytes.bin", std::string_view(reinterpret_cast<const char*>(expected.data()), expected.size()));

    const auto data = ReadFile(dir.Path() / "bytes.bin");

    ASSERT_TRUE(data.has_value());
    EXPECT_EQ(*data, expected);
}

TEST_F(FileHelpersTests, EmptyFileReadsAsEmptyData) {
    WriteRaw(dir.Path() / "empty.bin", "");
    const auto data = ReadFile(dir.Path() / "empty.bin");
    ASSERT_TRUE(data.has_value());
    EXPECT_TRUE(data->empty());
}

TEST_F(FileHelpersTests, ReadsLargeFiles) {
    std::vector<std::uint8_t> expected(3 * 1024 * 1024 + 7);
    for (std::size_t i = 0; i < expected.size(); ++i) expected[i] = static_cast<std::uint8_t>(i * 31u);
    ASSERT_TRUE(WriteFile(dir.Path() / "large.bin", expected));

    const auto data = ReadFile(dir.Path() / "large.bin");
    ASSERT_TRUE(data.has_value());
    EXPECT_EQ(*data, expected);
}

TEST_F(FileHelpersTests, ReadingAMissingFileFails) {
    EXPECT_FALSE(ReadFile(dir.Path() / "missing.bin").has_value());
}

TEST_F(FileHelpersTests, ReadingAFolderFails) {
    EXPECT_FALSE(ReadFile(dir.Path()).has_value());
}

TEST_F(FileHelpersTests, ReadsPathsWithNonAsciiCharacters) {
    const auto path = dir.Path() / fs::path(u8"ação.bin");
    ASSERT_TRUE(WriteFile(path, Bytes("acentos")));
    const auto data = ReadFile(path);
    ASSERT_TRUE(data.has_value());
    EXPECT_EQ(*data, Bytes("acentos"));
}

// ---------------------------------------------------------------- writing

TEST_F(FileHelpersTests, WriteAcceptsAnyContiguousBuffer) {
    const std::uint8_t array[] = {1, 2, 3, 4, 5};
    ASSERT_TRUE(WriteFile(dir.Path() / "array.bin", array));
    EXPECT_EQ(ReadFile(dir.Path() / "array.bin"), (std::vector<std::uint8_t>{1, 2, 3, 4, 5}));

    const std::vector<std::uint8_t> vector{9, 8, 7, 6};
    ASSERT_TRUE(WriteFile(dir.Path() / "part.bin", std::span(vector).subspan(1, 2)));
    EXPECT_EQ(ReadFile(dir.Path() / "part.bin"), (std::vector<std::uint8_t>{8, 7}));
}

TEST_F(FileHelpersTests, WriteThenReadRoundTripsEveryByte) {
    const auto expected = AllByteValues();
    ASSERT_TRUE(WriteFile(dir.Path() / "round.bin", expected));
    EXPECT_EQ(fs::file_size(dir.Path() / "round.bin"), expected.size());
    EXPECT_EQ(ReadFile(dir.Path() / "round.bin"), expected);
}

TEST_F(FileHelpersTests, WritingReplacesTheWholeOldContent) {
    ASSERT_TRUE(WriteFile(dir.Path() / "save.bin", Bytes("a much longer old content")));
    ASSERT_TRUE(WriteFile(dir.Path() / "save.bin", Bytes("new")));
    EXPECT_EQ(ReadFile(dir.Path() / "save.bin"), Bytes("new"));
}

TEST_F(FileHelpersTests, WritingEmptyDataCreatesAnEmptyFile) {
    ASSERT_TRUE(WriteFile(dir.Path() / "empty.bin", {}));
    EXPECT_TRUE(FileExists(dir.Path() / "empty.bin"));
    EXPECT_EQ(fs::file_size(dir.Path() / "empty.bin"), 0u);
}

TEST_F(FileHelpersTests, WritingCreatesMissingParentFolders) {
    const auto path = dir.Path() / "saves" / "slot1" / "game.sav";
    ASSERT_TRUE(WriteFile(path, Bytes("data")));
    EXPECT_EQ(ReadFile(path), Bytes("data"));
}

TEST_F(FileHelpersTests, SuccessfulWritesLeaveNoTemporaryFile) {
    ASSERT_TRUE(WriteFile(dir.Path() / "save.bin", Bytes("data")));
    ASSERT_TRUE(WriteFile(dir.Path() / "save.bin", Bytes("data2")));
    std::size_t entries = 0;
    for ([[maybe_unused]] const auto& entry : fs::directory_iterator(dir.Path())) ++entries;
    EXPECT_EQ(entries, 1u);
}

TEST_F(FileHelpersTests, WritingOverAFolderFailsAndKeepsIt) {
    fs::create_directories(dir.Path() / "folder" / "inside");
    EXPECT_FALSE(WriteFile(dir.Path() / "folder", Bytes("data")));
    EXPECT_TRUE(DirectoryExists(dir.Path() / "folder" / "inside"));
    EXPECT_FALSE(fs::exists(dir.Path() / "folder.tmp"));
}

TEST_F(FileHelpersTests, WritingUnderAFileFails) {
    WriteRaw(dir.Path() / "file.txt", "x");
    EXPECT_FALSE(WriteFile(dir.Path() / "file.txt" / "child.bin", Bytes("data")));
    EXPECT_EQ(ReadFile(dir.Path() / "file.txt"), Bytes("x"));
}

TEST_F(FileHelpersTests, WritingAPathWithoutFileNameFails) {
    EXPECT_FALSE(WriteFile("", Bytes("data")));
    EXPECT_FALSE(WriteFile(dir.Path() / "", Bytes("data")));
}

TEST_F(FileHelpersTests, FailedWriteKeepsTheOldFile) {
#ifdef _WIN32
    GTEST_SKIP() << "Folder write permissions work differently on Windows.";
#else
    const auto folder = dir.Path() / "locked";
    ASSERT_TRUE(WriteFile(folder / "save.bin", Bytes("old")));
    fs::permissions(folder, fs::perms::owner_write | fs::perms::group_write | fs::perms::others_write,
                    fs::perm_options::remove);
    if (std::ofstream probe(folder / "probe"); probe) {
        GTEST_SKIP() << "Running with permissions that ignore read-only folders (e.g. root).";
    }

    EXPECT_FALSE(WriteFile(folder / "save.bin", Bytes("new")));
    fs::permissions(folder, fs::perms::owner_write, fs::perm_options::add);
    EXPECT_EQ(ReadFile(folder / "save.bin"), Bytes("old"));
#endif
}
