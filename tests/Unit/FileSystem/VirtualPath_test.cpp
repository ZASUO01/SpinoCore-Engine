// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <gtest/gtest.h>
#include "FileSystem/Core/VirtualPath.h"
#include "SpinoCore/Logs/Logger.h"

using namespace SpinoCore::FileSystem::Core;

TEST(VirtualPathUnitTest, ShouldParseValidVirtualPath) {
    const auto parsed = ParseVirtualPath("proto://folder/TEST_F.txt");

    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(parsed->protocol, "proto");
    EXPECT_EQ(parsed->path, "folder/TEST_F.txt");
}

TEST(VirtualPathUnitTest, ShouldFailMalformedVirtualPath) {
    EXPECT_FALSE(ParseVirtualPath("").has_value());
    EXPECT_FALSE(ParseVirtualPath("folder/test.txt").has_value());
    EXPECT_FALSE(ParseVirtualPath("://folder/test.txt").has_value());
    EXPECT_FALSE(ParseVirtualPath("proto://").has_value());
    EXPECT_FALSE(IsValidVirtualPath("/folder/test.png"));
    EXPECT_FALSE(IsValidVirtualPath("folder//test.png"));
    EXPECT_FALSE(IsValidVirtualPath("folder/folder_1///test.obj"));
}

TEST(VirtualPathUnitTest, ShouldValidadePathsWithAllowedCharacters) {
    EXPECT_TRUE(IsValidVirtualPath("folder/test.txt"));
    EXPECT_TRUE(IsValidVirtualPath("folder/folder_1/file-1.txt"));
    EXPECT_TRUE(IsValidVirtualPath("file_v2.json"));
    EXPECT_TRUE(IsValidVirtualPath("folder.with.dots/file.txt"));
}

TEST(VirtualPathUnitTest, ShouldInvalidatePathWithForbiddenCharacters) {
    EXPECT_FALSE(IsValidVirtualPath("folder/test space.png"));
    EXPECT_FALSE(IsValidVirtualPath("folder\\backslash.png"));
    EXPECT_FALSE(IsValidVirtualPath("folder/test@file.obj"));
    EXPECT_FALSE(IsValidVirtualPath("folder/file#1.txt"));
    EXPECT_FALSE(IsValidVirtualPath("folder/file!.json"));
}