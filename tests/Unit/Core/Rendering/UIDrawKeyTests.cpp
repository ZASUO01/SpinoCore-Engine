// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <cstdint>
#include <gtest/gtest.h>
#include "Core/Rendering/Drawer/DrawCommands.h"

namespace {
    using SpinoCore::Core::Rendering::MakeUIDrawKey;
    using SpinoCore::Core::Rendering::UIKeyMaxLayer;

    TEST(UIDrawKeyTests, TheLayerDecidesFirst) {
        EXPECT_LT(MakeUIDrawKey(1, 2'000'000'000, 0xFFFF), MakeUIDrawKey(2, 0, 0));
    }

    TEST(UIDrawKeyTests, ThenTheDrawOrder) {
        EXPECT_LT(MakeUIDrawKey(1, 10, 0xFFFF), MakeUIDrawKey(1, 11, 0));
    }

    TEST(UIDrawKeyTests, TheTextureOnlyBreaksTies) {
        EXPECT_LT(MakeUIDrawKey(1, 10, 3), MakeUIDrawKey(1, 10, 4));
        EXPECT_EQ(MakeUIDrawKey(1, 10, 4), MakeUIDrawKey(1, 10, 4));
    }

    TEST(UIDrawKeyTests, LargeTextureIdsNeverOvertakeTheDrawOrder) {
        EXPECT_LT(MakeUIDrawKey(1, 10, 0xFFFF'FFFFU), MakeUIDrawKey(1, 11, 0));
        EXPECT_EQ(MakeUIDrawKey(1, 10, 0x1'0005U), MakeUIDrawKey(1, 10, 5));
    }

    TEST(UIDrawKeyTests, FieldsLandInTheirOwnBits) {
        EXPECT_EQ(MakeUIDrawKey(0x1234, 0x0ABC'DEF0, 0x5678), 0x1234'0ABC'DEF0'5678ULL);
    }

    TEST(UIDrawKeyTests, LayersOutsideTheRangeAreClamped) {
        EXPECT_EQ(MakeUIDrawKey(UIKeyMaxLayer + 10, 0, 0), MakeUIDrawKey(UIKeyMaxLayer, 0, 0));
        EXPECT_EQ(MakeUIDrawKey(-3, 5, 1), MakeUIDrawKey(0, 5, 1));
    }

    TEST(UIDrawKeyTests, KeysCanBeBuiltAtCompileTime) {
        static_assert(MakeUIDrawKey(1, 0, 0) == std::uint64_t{1} << 48U);
        SUCCEED();
    }
}
