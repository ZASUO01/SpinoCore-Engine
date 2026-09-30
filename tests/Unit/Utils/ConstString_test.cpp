// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <gtest/gtest.h>
#include <string_view>
#include <algorithm>
#include "Utils/ConstString.h"

using namespace SpinoCore::Utils;

constexpr ConstString compileTimeStr1("Spino");
constexpr ConstString compileTimeStr2("Core");
constexpr auto compileTimeConcat = compileTimeStr1 + compileTimeStr2;

static_assert(compileTimeStr1.size() == 5);
static_assert(compileTimeStr2.size() == 4);
static_assert(compileTimeConcat.size() == 9);
static_assert(compileTimeConcat.buffer[0] == 'S');
static_assert(compileTimeConcat.buffer[5] == 'C');
static_assert(compileTimeConcat.buffer[9] == '\0');

TEST(ConstStringUnitTest, ShouldInitializeProperlyAndDeduceSize) {
    constexpr ConstString str("Hello");

    EXPECT_EQ(str.size(), 5);
    EXPECT_STREQ(str.c_str(), "Hello");
    EXPECT_STREQ(str.data(), "Hello");
}

TEST(ConstStringTest, ShouldExplicitlyConvertToStringView) {
    constexpr ConstString str("Engine");

    auto sv = static_cast<std::string_view>(str); // NOLINT

    EXPECT_EQ(sv.length(), 6);
    EXPECT_EQ(sv, "Engine");
}

TEST(ConstStringTest, ShouldConcatenateTwoStringsCorrectly) {
    constexpr ConstString a("Virtual");
    constexpr ConstString b("FileSystem");
    constexpr auto result = a + b;

    EXPECT_EQ(result.size(), 17);
    EXPECT_STREQ(result.c_str(), "VirtualFileSystem");
}

TEST(ConstStringTest, ShouldSupportStlIteratorsAndAlgorithms) {
    constexpr ConstString str("abc");

    std::string copy;
    for (const char c : str) {
        copy += c;
    }
    EXPECT_EQ(copy, "abc");

    constexpr ConstString target("test");
    constexpr bool has_e = std::ranges::any_of(target, [](const char c) { return c == 'e'; });
    EXPECT_TRUE(has_e);
}

TEST(ConstStringTest, ShouldHandleEmptyStrings) {
    constexpr ConstString empty("");

    EXPECT_EQ(empty.size(), 0);
    EXPECT_STREQ(empty.c_str(), "");
    EXPECT_EQ(empty.begin(), empty.end());

    constexpr ConstString str("Data");
    constexpr auto concatLeft = empty + str;
    constexpr auto concatRight = str + empty;

    EXPECT_STREQ(concatLeft.c_str(), "Data");
    EXPECT_STREQ(concatRight.c_str(), "Data");
}