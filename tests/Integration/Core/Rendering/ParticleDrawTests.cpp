// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <optional>
#include <gtest/gtest.h>
#include "../../Graphics/GLTestContext.h"
#include "Core/Rendering/Drawer/DrawCommands.h"
#include "Core/Rendering/Drawer/Drawer.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Core::Rendering::Drawer;
    using SpinoCore::Core::Rendering::ParticleBatch;
    using SpinoCore::Core::Rendering::ParticleInstance;
    using SpinoCore::Math::Matrix4f;
    using SpinoCore::Math::Vector3f;
    using SpinoCore::Math::Vector4f;
    using SpinoCore::Tests::GLTestContext;
    using SpinoCore::Tests::Pixel;

    class ParticleDrawTests : public GLTestContext {
    protected:
        void SetUp() override {
            GLTestContext::SetUp();
            if (HasFatalFailure()) return;
            drawer.emplace();
        }

        void TearDown() override {
            drawer.reset();
            GLTestContext::TearDown();
        }

        void DrawParticleAt(const Vector3f& position) {
            auto& list = drawer->GetParticleDrawList();
            list.instances.push_back(ParticleInstance{.position = position, .size = 1.0f, .color = Vector4f{1.0f, 0.0f, 0.0f, 1.0f}, .rotation = 0.0f});
            list.batches.push_back(ParticleBatch{.first = 0, .count = 1, .additive = true});
            drawer->DrawParticles(Matrix4f::Identity(), Matrix4f::Identity());
        }

        std::optional<Drawer> drawer{};
    };

    TEST_F(ParticleDrawTests, AParticleIsDrawnAsAColoredBillboard) {
        DrawParticleAt(Vector3f::Zero());

        EXPECT_EQ(ReadPixel(8, 8), (Pixel{255, 0, 0, 255}));
    }

    TEST_F(ParticleDrawTests, TheBillboardFadesTowardsItsCorners) {
        DrawParticleAt(Vector3f::Zero());

        EXPECT_EQ(ReadPixel(4, 4)[0], 0);
    }

    TEST_F(ParticleDrawTests, DrawingConsumesTheParticles) {
        DrawParticleAt(Vector3f::Zero());

        EXPECT_TRUE(drawer->GetParticleDrawList().instances.empty());
    }
}
