// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <gtest/gtest.h>
#include "SpinoCore/Config/ConfigEntry.h"

using namespace SpinoCore::Config::Entry;

TEST(ConfigKey, HashValueIsDeterministic) {
    constexpr ConfigKey keyA("ValueX");
    constexpr ConfigKey keyB("ValueX");
    constexpr ConfigKey keyC("ValueY");

    EXPECT_EQ(keyA.hash, keyB.hash);
    EXPECT_NE(keyA.hash, keyC.hash);
}

TEST(ConfigKey, FNV1AlgorithmReturnsCorrectHash) {
    constexpr ConfigKey keyA("ValueX");

    EXPECT_EQ(keyA.hash, 2790082998u);
}