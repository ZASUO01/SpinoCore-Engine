// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include "Utils/JSONHelpers.h"

namespace {
    using SpinoCore::Utils::JSON;
    using SpinoCore::Utils::ParseJSON;
    using Fields = std::vector<std::string>;

    [[nodiscard]] std::vector<std::uint8_t> Bytes(const std::string_view text) {
        return {text.begin(), text.end()};
    }

    constexpr std::string_view Manifest = R"({
        "name": "demo",
        "version": 2,
        "optional": null,
        "texture": { "path": "hero.png", "filter": { "min": "linear" } },
        "list": [ { "id": "a" } ],
        "a.b": true
    })";

    TEST(JSONHelpersTests, ParsesValidJSONWithoutRequiredFields) {
        const auto json = ParseJSON(Bytes(Manifest));

        ASSERT_TRUE(json.has_value());
        EXPECT_EQ(json->at("name"), "demo");
        EXPECT_EQ(json->at("texture").at("filter").at("min"), "linear");
    }

    TEST(JSONHelpersTests, NonObjectRootsParseWhenNoFieldIsRequired) {
        const auto json = ParseJSON(Bytes("[1, 2, 3]"));

        ASSERT_TRUE(json.has_value());
        EXPECT_EQ(json->size(), 3u);
    }

    TEST(JSONHelpersTests, InvalidSyntaxFails) {
        EXPECT_FALSE(ParseJSON(Bytes(R"({"name": "demo",})")).has_value());
        EXPECT_FALSE(ParseJSON(Bytes(R"({"name": )")).has_value());
        EXPECT_FALSE(ParseJSON(Bytes("not json")).has_value());
    }

    TEST(JSONHelpersTests, EmptyDataFails) {
        EXPECT_FALSE(ParseJSON({}).has_value());
        EXPECT_FALSE(ParseJSON(Bytes("   \n")).has_value());
    }

    TEST(JSONHelpersTests, TrailingContentFails) {
        EXPECT_FALSE(ParseJSON(Bytes(R"({"a": 1} {"b": 2})")).has_value());
    }

    TEST(JSONHelpersTests, InvalidUTF8Fails) {
        std::vector<std::uint8_t> data = Bytes(R"({"name": "x"})");
        data[10] = 0xFF;

        EXPECT_FALSE(ParseJSON(data).has_value());
    }

    TEST(JSONHelpersTests, UTF8ByteOrderMarkIsAccepted) {
        std::vector<std::uint8_t> data{0xEF, 0xBB, 0xBF};
        const auto text = Bytes(R"({"name": "demo"})");
        data.insert(data.end(), text.begin(), text.end());

        const auto json = ParseJSON(data, Fields{"name"});

        ASSERT_TRUE(json.has_value());
        EXPECT_EQ(json->at("name"), "demo");
    }

    TEST(JSONHelpersTests, TopLevelFieldsArePresent) {
        EXPECT_TRUE(ParseJSON(Bytes(Manifest), Fields{"name", "version", "texture"}).has_value());
    }

    TEST(JSONHelpersTests, NestedFieldsAreSeparatedBySlashes) {
        EXPECT_TRUE(ParseJSON(Bytes(Manifest), Fields{"texture/path", "texture/filter/min"}).has_value());
    }

    TEST(JSONHelpersTests, MissingTopLevelFieldFails) {
        EXPECT_FALSE(ParseJSON(Bytes(Manifest), Fields{"author"}).has_value());
    }

    TEST(JSONHelpersTests, MissingNestedFieldFails) {
        EXPECT_FALSE(ParseJSON(Bytes(Manifest), Fields{"texture/size"}).has_value());
        EXPECT_FALSE(ParseJSON(Bytes(Manifest), Fields{"texture/filter/mag"}).has_value());
    }

    TEST(JSONHelpersTests, OneMissingFieldAmongManyFails) {
        EXPECT_FALSE(ParseJSON(Bytes(Manifest), Fields{"name", "texture/path", "missing", "version"}).has_value());
    }

    TEST(JSONHelpersTests, FieldsCannotGoThroughValuesThatAreNotObjects) {
        EXPECT_FALSE(ParseJSON(Bytes(Manifest), Fields{"name/length"}).has_value());
        EXPECT_FALSE(ParseJSON(Bytes(Manifest), Fields{"texture/path/extension"}).has_value());
    }

    TEST(JSONHelpersTests, ArraysAreNotWalkedInto) {
        EXPECT_TRUE(ParseJSON(Bytes(Manifest), Fields{"list"}).has_value());
        EXPECT_FALSE(ParseJSON(Bytes(Manifest), Fields{"list/0"}).has_value());
        EXPECT_FALSE(ParseJSON(Bytes(Manifest), Fields{"list/0/id"}).has_value());
    }

    TEST(JSONHelpersTests, NonObjectRootHasNoFields) {
        EXPECT_FALSE(ParseJSON(Bytes(R"(["name"])"), Fields{"name"}).has_value());
        EXPECT_FALSE(ParseJSON(Bytes(R"("name")"), Fields{"name"}).has_value());
    }

    TEST(JSONHelpersTests, NullValuesCountAsPresent) {
        EXPECT_TRUE(ParseJSON(Bytes(Manifest), Fields{"optional"}).has_value());
    }

    TEST(JSONHelpersTests, FieldNamesAreCaseSensitive) {
        EXPECT_FALSE(ParseJSON(Bytes(Manifest), Fields{"Name"}).has_value());
    }

    TEST(JSONHelpersTests, OnlySlashesSplitFieldNames) {
        EXPECT_TRUE(ParseJSON(Bytes(Manifest), Fields{"a.b"}).has_value());
    }

    TEST(JSONHelpersTests, MalformedFieldNamesFail) {
        for (const std::string field : {"", "/", "/name", "name/", "texture//path", "texture/path/"}) {
            EXPECT_FALSE(ParseJSON(Bytes(Manifest), Fields{field}).has_value()) << '"' << field << '"';
        }
    }

    TEST(JSONHelpersTests, EmptyKeysAreNotReachable) {
        EXPECT_FALSE(ParseJSON(Bytes(R"({"": 1})"), Fields{""}).has_value());
    }

    TEST(JSONHelpersTests, TheParsedDocumentIsReturnedWhole) {
        const auto json = ParseJSON(Bytes(Manifest), Fields{"texture/path"});

        ASSERT_TRUE(json.has_value());
        EXPECT_EQ(*json, JSON::parse(Manifest));
    }
}
