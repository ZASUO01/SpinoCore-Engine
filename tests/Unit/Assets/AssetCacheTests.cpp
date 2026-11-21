// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <gtest/gtest.h>
#include "Assets/AssetCache.h"

namespace {
    using SpinoCore::Assets::AssetCache;

    // Stand-in for a real asset type such as a texture.
    struct FakeTexture final {
        int width{0};
        int height{0};
    };

    // Counts live instances, to check when the cache destroys its assets.
    struct Tracked final {
        static inline int alive = 0;
        Tracked() { ++alive; }
        ~Tracked() { --alive; }
        Tracked(const Tracked&) = delete;
        Tracked& operator=(const Tracked&) = delete;
    };

    [[nodiscard]] std::unique_ptr<const FakeTexture> Texture(const int width, const int height) {
        return std::make_unique<const FakeTexture>(FakeTexture{width, height});
    }
}

// ---------------------------------------------------------------- adding and getting

TEST(AssetCacheTests, StartsEmptyWithoutDefault) {
    const AssetCache<FakeTexture> cache;
    EXPECT_TRUE(cache.IsEmpty());
    EXPECT_EQ(cache.Size(), 0u);
    EXPECT_FALSE(cache.HasDefault());
}

TEST(AssetCacheTests, AddTakesOwnershipAndReturnsTheStoredAsset) {
    AssetCache<FakeTexture> cache;
    auto texture = Texture(32, 16);
    const auto* raw = texture.get();

    const auto& added = cache.Add("player", std::move(texture));

    EXPECT_EQ(&added, raw);
    EXPECT_EQ(cache.Size(), 1u);
    EXPECT_FALSE(cache.IsEmpty());
}

TEST(AssetCacheTests, GetReturnsTheAssetStoredUnderEachId) {
    AssetCache<FakeTexture> cache;
    auto player = Texture(32, 32);
    auto enemy = Texture(64, 48);
    const auto* playerRaw = player.get();
    const auto* enemyRaw = enemy.get();
    (void)cache.Add("player", std::move(player));
    (void)cache.Add("enemy", std::move(enemy));

    EXPECT_EQ(&cache.Get("player"), playerRaw);
    EXPECT_EQ(&cache.Get("enemy"), enemyRaw);
    EXPECT_EQ(cache.Get("enemy").width, 64);
}

TEST(AssetCacheTests, LookupsAcceptEveryStringKind) {
    AssetCache<FakeTexture> cache;
    (void)cache.Add("player", Texture(1, 1));

    const std::string asString = "player";
    const std::string_view asView = asString;
    EXPECT_TRUE(cache.Contains("player"));
    EXPECT_TRUE(cache.Contains(asString));
    EXPECT_TRUE(cache.Contains(asView));
    // A view that is not null-terminated must still match only its own characters.
    EXPECT_TRUE(cache.Contains(std::string_view("player-two").substr(0, 6)));
    EXPECT_FALSE(cache.Contains(std::string_view("player-two")));
}

TEST(AssetCacheTests, IdsAreCaseSensitive) {
    AssetCache<FakeTexture> cache;
    (void)cache.Add("Player", Texture(1, 1));
    EXPECT_TRUE(cache.Contains("Player"));
    EXPECT_FALSE(cache.Contains("player"));
}

TEST(AssetCacheTests, GetOfAMissingIdThrowsOutOfRange) {
    AssetCache<FakeTexture> cache;
    (void)cache.Add("player", Texture(1, 1));
    EXPECT_THROW((void)cache.Get("enemy"), std::out_of_range);
}

TEST(AssetCacheTests, MissingIdErrorNamesTheId) {
    const AssetCache<FakeTexture> cache;
    try {
        (void)cache.Get("boss");
        FAIL() << "Expected std::out_of_range";
    } catch (const std::out_of_range& error) {
        EXPECT_NE(std::string(error.what()).find("\"boss\""), std::string::npos);
    }
}

TEST(AssetCacheTests, AddRejectsNullAssets) {
    AssetCache<FakeTexture> cache;
    EXPECT_THROW((void)cache.Add("player", nullptr), std::invalid_argument);
    EXPECT_TRUE(cache.IsEmpty());
}

TEST(AssetCacheTests, AddRejectsEmptyIds) {
    AssetCache<FakeTexture> cache;
    EXPECT_THROW((void)cache.Add("", Texture(1, 1)), std::invalid_argument);
    EXPECT_TRUE(cache.IsEmpty());
}

TEST(AssetCacheTests, DuplicatedIdsThrowAndKeepTheFirstAsset) {
    AssetCache<FakeTexture> cache;
    auto first = Texture(1, 1);
    const auto* firstRaw = first.get();
    (void)cache.Add("player", std::move(first));

    EXPECT_THROW((void)cache.Add("player", Texture(2, 2)), std::invalid_argument);
    EXPECT_EQ(&cache.Get("player"), firstRaw);
    EXPECT_EQ(cache.Size(), 1u);
}

TEST(AssetCacheTests, RejectedAssetsAreDestroyed) {
    ASSERT_EQ(Tracked::alive, 0);
    {
        AssetCache<Tracked> cache;
        (void)cache.Add("a", std::make_unique<const Tracked>());
        EXPECT_THROW((void)cache.Add("a", std::make_unique<const Tracked>()), std::invalid_argument);
        EXPECT_THROW((void)cache.Add("", std::make_unique<const Tracked>()), std::invalid_argument);
        EXPECT_EQ(Tracked::alive, 1);
    }
    EXPECT_EQ(Tracked::alive, 0);
}

// ---------------------------------------------------------------- default asset

TEST(AssetCacheTests, DefaultCanBeSetInTheConstructor) {
    auto fallback = Texture(2, 2);
    const auto* raw = fallback.get();
    const AssetCache<FakeTexture> cache{std::move(fallback)};
    EXPECT_TRUE(cache.HasDefault());
    EXPECT_EQ(&cache.GetDefault(), raw);
}

TEST(AssetCacheTests, NullDefaultIsRejected) {
    AssetCache<FakeTexture> cache;
    EXPECT_THROW(cache.SetDefault(nullptr), std::invalid_argument);
    EXPECT_THROW((AssetCache<FakeTexture>{nullptr}), std::invalid_argument);
    EXPECT_FALSE(cache.HasDefault());
}

TEST(AssetCacheTests, GetDefaultWithoutOneThrows) {
    const AssetCache<FakeTexture> cache;
    EXPECT_THROW((void)cache.GetDefault(), std::logic_error);
}

TEST(AssetCacheTests, GetOrDefaultReturnsTheCachedAsset) {
    AssetCache<FakeTexture> cache{Texture(1, 1)};
    const FakeTexture& stored = cache.Add("hero", Texture(64, 32));

    EXPECT_EQ(&cache.GetOrDefault("hero"), &stored);
}

TEST(AssetCacheTests, GetOrDefaultFallsBackToTheDefaultForMissingIds) {
    AssetCache<FakeTexture> cache{Texture(1, 1)};
    cache.Add("hero", Texture(64, 32));

    EXPECT_EQ(&cache.GetOrDefault("missing"), &cache.GetDefault());
}

TEST(AssetCacheTests, GetOrDefaultWithoutADefaultThrows) {
    const AssetCache<FakeTexture> cache;

    EXPECT_THROW((void)cache.GetOrDefault("missing"), std::logic_error);
}

TEST(AssetCacheTests, SetDefaultReplacesAndDestroysThePreviousOne) {
    ASSERT_EQ(Tracked::alive, 0);
    {
        AssetCache<Tracked> cache{std::make_unique<const Tracked>()};
        EXPECT_EQ(Tracked::alive, 1);

        auto replacement = std::make_unique<const Tracked>();
        const auto* raw = replacement.get();
        cache.SetDefault(std::move(replacement));

        EXPECT_EQ(Tracked::alive, 1);
        EXPECT_EQ(&cache.GetDefault(), raw);
    }
    EXPECT_EQ(Tracked::alive, 0);
}

TEST(AssetCacheTests, DefaultIsNotAnEntry) {
    const AssetCache<FakeTexture> cache{Texture(2, 2)};
    EXPECT_TRUE(cache.IsEmpty());
    EXPECT_FALSE(cache.Contains(""));
}

// ---------------------------------------------------------------- removal and ownership

TEST(AssetCacheTests, RemoveDeletesOnlyThatId) {
    AssetCache<FakeTexture> cache;
    (void)cache.Add("player", Texture(1, 1));
    (void)cache.Add("enemy", Texture(1, 1));

    EXPECT_TRUE(cache.Remove(std::string_view("player")));
    EXPECT_FALSE(cache.Contains("player"));
    EXPECT_TRUE(cache.Contains("enemy"));
    EXPECT_FALSE(cache.Remove("player"));
    EXPECT_EQ(cache.Size(), 1u);
}

TEST(AssetCacheTests, RemovedIdCanBeAddedAgain) {
    AssetCache<FakeTexture> cache;
    (void)cache.Add("player", Texture(1, 1));
    (void)cache.Remove("player");
    auto replacement = Texture(8, 8);
    const auto* raw = replacement.get();
    (void)cache.Add("player", std::move(replacement));
    EXPECT_EQ(&cache.Get("player"), raw);
}

TEST(AssetCacheTests, ClearRemovesAssetsButKeepsTheDefault) {
    auto fallback = Texture(2, 2);
    const auto* raw = fallback.get();
    AssetCache<FakeTexture> cache{std::move(fallback)};
    (void)cache.Add("player", Texture(1, 1));

    cache.Clear();

    EXPECT_TRUE(cache.IsEmpty());
    EXPECT_TRUE(cache.HasDefault());
    EXPECT_EQ(&cache.GetDefault(), raw);
}

TEST(AssetCacheTests, CacheOwnsItsAssets) {
    ASSERT_EQ(Tracked::alive, 0);
    {
        AssetCache<Tracked> cache{std::make_unique<const Tracked>()};
        (void)cache.Add("a", std::make_unique<const Tracked>());
        (void)cache.Add("b", std::make_unique<const Tracked>());
        EXPECT_EQ(Tracked::alive, 3);

        (void)cache.Remove("a");
        EXPECT_EQ(Tracked::alive, 2);

        cache.Clear();
        EXPECT_EQ(Tracked::alive, 1);
    }
    EXPECT_EQ(Tracked::alive, 0);
}

TEST(AssetCacheTests, CachesOfDifferentTypesAreIndependent) {
    AssetCache<FakeTexture> textures;
    AssetCache<std::string> texts;
    (void)textures.Add("intro", Texture(1, 1));
    (void)texts.Add("intro", std::make_unique<const std::string>("Hello"));

    EXPECT_EQ(textures.Get("intro").width, 1);
    EXPECT_EQ(texts.Get("intro"), "Hello");
}
