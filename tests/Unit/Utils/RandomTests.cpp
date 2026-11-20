// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <set>
#include <thread>
#include <vector>
#include <gtest/gtest.h>
#include "SpinoCore/Utils/Random.h"

namespace {
    namespace Random = SpinoCore::Utils::Random;

    constexpr int Draws = 2000;

    [[nodiscard]] std::vector<int> DrawInts(const std::uint32_t seed) {
        Random::Seed(seed);
        std::vector<int> values;
        for (int i = 0; i < 32; ++i) values.push_back(Random::GetIntRange(0, 1'000'000));
        return values;
    }

    // Every test seeds the engine itself, so results never depend on test order or on the clock.
    TEST(RandomTests, TheSameSeedRepeatsTheSameSequence) {
        EXPECT_EQ(DrawInts(1234), DrawInts(1234));
    }

    TEST(RandomTests, DifferentSeedsGiveDifferentSequences) {
        EXPECT_NE(DrawInts(1), DrawInts(2));
    }

    TEST(RandomTests, IntegersStayWithinTheInclusiveRange) {
        Random::Seed(42);

        for (int i = 0; i < Draws; ++i) {
            const int value = Random::GetIntRange(-3, 3);
            ASSERT_GE(value, -3);
            ASSERT_LE(value, 3);
        }
    }

    TEST(RandomTests, IntegersReachBothEndsOfTheRange) {
        Random::Seed(7);
        std::set<int> seen;

        for (int i = 0; i < Draws; ++i) seen.insert(Random::GetIntRange(1, 6));

        EXPECT_EQ(seen, (std::set<int>{1, 2, 3, 4, 5, 6}));
    }

    TEST(RandomTests, IntegersAreRoughlyUniform) {
        Random::Seed(99);
        std::array<int, 4> counts{};

        for (int i = 0; i < 4000; ++i) ++counts[static_cast<std::size_t>(Random::GetIntRange(0, 3))];

        // About 1000 each; the bounds are far outside what a fair generator produces.
        for (const int count : counts) {
            EXPECT_GT(count, 800);
            EXPECT_LT(count, 1200);
        }
    }

    TEST(RandomTests, ASingleValueRangeAlwaysReturnsThatValue) {
        Random::Seed(5);

        EXPECT_EQ(Random::GetIntRange(9, 9), 9);
        EXPECT_FLOAT_EQ(Random::GetFloatRange(2.5f, 2.5f), 2.5f);
    }

    TEST(RandomTests, WorksWithOtherIntegerTypes) {
        Random::Seed(11);

        const auto big = Random::GetIntRange<std::uint64_t>(10'000'000'000ull, 10'000'000'010ull);
        const auto small = Random::GetIntRange<short>(-2, 2);

        EXPECT_GE(big, 10'000'000'000ull);
        EXPECT_LE(big, 10'000'000'010ull);
        EXPECT_GE(small, -2);
        EXPECT_LE(small, 2);
    }

    TEST(RandomTests, FloatsStayWithinTheHalfOpenRange) {
        Random::Seed(42);

        for (int i = 0; i < Draws; ++i) {
            const float value = Random::GetFloatRange(-1.0f, 1.0f);
            ASSERT_GE(value, -1.0f);
            ASSERT_LT(value, 1.0f);
        }
    }

    TEST(RandomTests, FloatsSpreadAcrossTheRange) {
        Random::Seed(3);
        float lowest = 1.0f;
        float highest = 0.0f;
        double sum = 0.0;

        for (int i = 0; i < Draws; ++i) {
            const auto value = Random::GetFloatRange(0.0f, 1.0f);
            lowest = std::min(lowest, value);
            highest = std::max(highest, value);
            sum += value;
        }

        EXPECT_LT(lowest, 0.05f);
        EXPECT_GT(highest, 0.95f);
        EXPECT_NEAR(sum / Draws, 0.5, 0.05);
    }

    TEST(RandomTests, DoublesAreSupported) {
        Random::Seed(8);

        const double value = Random::GetFloatRange(10.0, 20.0);

        EXPECT_GE(value, 10.0);
        EXPECT_LT(value, 20.0);
    }

    // The engine is thread_local: seeding or drawing on one thread does not disturb another.
    TEST(RandomTests, EachThreadHasItsOwnGenerator) {
        const std::vector<int> expected = DrawInts(2024);

        Random::Seed(2024);
        std::vector<int> mainThread;
        mainThread.push_back(Random::GetIntRange(0, 1'000'000));

        std::vector<int> otherThread;
        std::thread worker([&otherThread] { otherThread = DrawInts(2024); });
        worker.join();

        for (int i = 1; i < 32; ++i) mainThread.push_back(Random::GetIntRange(0, 1'000'000));

        EXPECT_EQ(mainThread, expected);
        EXPECT_EQ(otherThread, expected);
    }
}
