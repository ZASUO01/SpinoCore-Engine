// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Utils/JSON/JSONUtils.h"
#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <string_view>
#include <nlohmann/json.hpp>

using namespace SpinoCore::Utils::JSON;

namespace {
    std::vector<uint8_t> MakeBytes(const std::string_view str) {
        return {str.begin(), str.end()};
    }
}

TEST(JSONUtilsTest, ShouldFailOnEmptyBuffer) {
    constexpr std::vector<uint8_t> emptyBuffer;
    const auto result = ParseJSONFromBytes(emptyBuffer);

    EXPECT_FALSE(result.has_value());
}

TEST(JSONUtilsTest, ShouldFailOnMalformedJson) {
    const auto badJson = MakeBytes(R"({ "name": "SpinoEngine", "version": 1.0 )");
    const auto result = ParseJSONFromBytes(badJson);

    EXPECT_FALSE(result.has_value());
}

TEST(JSONUtilsTest, ShouldParseValidJsonWhenNoFieldsAreRequired) {
    const auto validJson = MakeBytes(R"({ "system": "VFS", "active": true })");
    const auto result = ParseJSONFromBytes(validJson);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["system"], "VFS");
    EXPECT_TRUE((*result)["active"].get<bool>());
}

TEST(JSONUtilsTest, ShouldSucceedWhenFlatRequiredFieldsExist) {
    const auto validJson = MakeBytes(R"({ "window": { "width": 1920, "height": 1080 }, "fullscreen": true })");
    const std::vector<std::string> req = {"window", "fullscreen"};

    const auto result = ParseJSONFromBytes(validJson, req);
    ASSERT_TRUE(result.has_value());
}

TEST(JSONUtilsTest, ShouldFailWhenFlatRequiredFieldIsMissing) {
    const auto validJson = MakeBytes(R"({ "window": { "width": 1920 } })");
    const std::vector<std::string> req = {"window", "vsync"};

    const auto result = ParseJSONFromBytes(validJson, req);
    EXPECT_FALSE(result.has_value());
}

TEST(JSONUtilsTest, ShouldSucceedWhenNestedRequiredFieldsExist) {
    const auto validJson = MakeBytes(R"({
        "engine": {
            "graphics": {
                "opengl": {
                    "version": 4.6
                }
            }
        }
    })");

    const std::vector<std::string> req = {"engine/graphics/opengl/version"}; // NOLINT
    const auto result = ParseJSONFromBytes(validJson, req);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["engine"]["graphics"]["opengl"]["version"], 4.6);
}

TEST(JSONUtilsTest, ShouldFailWhenTraversingNonObjectNode) {
    const auto validJson = MakeBytes(R"({
        "graphics": [1920, 1080],
        "name": "SpinoCore"
    })");

    const std::vector<std::string> req1 = {"graphics/width"};
    EXPECT_FALSE(ParseJSONFromBytes(validJson, req1).has_value());

    const std::vector<std::string> req2 = {"name/first"};
    EXPECT_FALSE(ParseJSONFromBytes(validJson, req2).has_value());
}

TEST(JSONUtilsTest, ShouldEnforceMaxDepthLimitOfFive) {
    const auto deepJson = MakeBytes(R"({
        "l1": { "l2": { "l3": { "l4": { "l5": { "l6": "too_deep" } } } } }
    })");

    const std::vector<std::string> limitReq = {"l1/l2/l3/l4/l5"};
    EXPECT_TRUE(ParseJSONFromBytes(deepJson, limitReq).has_value());

    const std::vector<std::string> exceededReq = {"l1/l2/l3/l4/l5/l6"}; // NOLINT
    EXPECT_FALSE(ParseJSONFromBytes(deepJson, exceededReq).has_value());
}