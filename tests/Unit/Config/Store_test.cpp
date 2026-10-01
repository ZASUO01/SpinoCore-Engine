// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <gtest/gtest.h>
#include "SpinoCore/Config/Store.h"

using namespace SpinoCore::Config;

namespace {
    class StoreTest : public testing::Test {
    protected:
        std::unique_ptr<Store> config;

        void SetUp() override {
            config = std::make_unique<Store>();

            config->Set("Vsync", true);
            config->Set("Volume", 0.8f);
            config->Set("MaxFPS", 60);
            config->Set("RendererContext", "OpenGL");
        }

        void TearDown() override {
            config.reset();
        }
    };
}

TEST_F(StoreTest, GetReturnsCorrectValuesOnSuccess) {
    EXPECT_EQ(config->Get("Vsync", false), true);
    EXPECT_EQ(config->Get("Volume", 1.0f), 0.8f);
    EXPECT_EQ(config->Get("MaxFPS", 120), 60);
}

TEST_F(StoreTest, GetReturnsDefaultValuesOnFailure) {
    EXPECT_EQ(config->Get("invalid-key", 0), 0);
}

TEST_F(StoreTest, StringOverloadReturnsExpectedValues) {
    EXPECT_EQ(config->Get("RendererContext", "Vulkan"), "OpenGL");
    EXPECT_EQ(config->Get("invalid-renderer", "DirectX"), "DirectX");
}

TEST_F(StoreTest, GetReturnsDefaultValuesOnTypeMismatch) {
    EXPECT_EQ(config->Get("Volume", "Vulkan"), "Vulkan");
    EXPECT_EQ(config->Get<int>("RendererContext", 0), 0);
    EXPECT_EQ(config->Get<std::string>("MaxFPS", "Fallback"), "Fallback");
}

TEST_F(StoreTest, ReadonlyFlagsPreventsChange) {
    config->Set("PhysicsGravity", 9.81f, true);

    config->Set("PhysicsGravity", 0.0f);
    EXPECT_FLOAT_EQ(config->Get<float>("PhysicsGravity", 0.0f), 9.81f);

    config->Set("PhysicsGravity", 100.0f, false);
    EXPECT_FLOAT_EQ(config->Get<float>("PhysicsGravity", 0.0f), 9.81f);
}

TEST_F(StoreTest, ReadOnlyFlagsPermitsFirstSet) {
    config->Set("CoreThreadCount", 8, true);
    EXPECT_EQ(config->Get<int>("CoreThreadCount", 0), 8);
}