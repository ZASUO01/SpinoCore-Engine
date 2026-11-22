// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <gtest/gtest.h>
#include "../../Graphics/GLTestContext.h"
#include "Core/Rendering/Drawer/DebugShapes.h"
#include "Core/Rendering/Renderer.h"
#include "Graphics/FrameBuffer.h"
#include "SpinoCore/Core/Lighting/GlobalLight.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Core::Lighting::GlobalLight;
    using SpinoCore::Core::Rendering::AddDebugLine;
    using SpinoCore::Graphics::FrameBuffer;
    using SpinoCore::Graphics::FrameBufferFormat;
    using SpinoCore::Math::Matrix4f;
    using SpinoCore::Math::Vector3f;
    using SpinoCore::Math::Vector4f;
    using SpinoCore::Tests::GLTestContext;
    using SpinoCore::Tests::Pixel;

    constexpr Pixel Red{255, 0, 0, 255};

    constexpr Vector4f RedColor{1.0f, 0.0f, 0.0f, 1.0f};

    class DebugDrawTests : public GLTestContext {
    protected:
        [[nodiscard]] Pixel DrawnCenter(const bool debugColliders) {
            renderer.reset();
            renderer.emplace(window->GetSDLWindow(), window->GetSize(), bus, GlobalLight{}, debugColliders);
            FrameBuffer target{{.width = TargetSize, .height = TargetSize, .format = FrameBufferFormat::ColorWithDepthRBO}};
            target.Bind();
            renderer->SetCamera(Matrix4f::Identity(), Matrix4f::Identity());
            AddDebugLine(renderer->GetDebugDrawList(), Vector3f{0.0625f, -0.5f, 0.0f}, Vector3f{0.0625f, 0.5f, 0.0f}, RedColor);

            renderer->Draw();

            const Pixel center = ReadPixel(8, 8);
            target.Unbind();
            return center;
        }
    };

    TEST_F(DebugDrawTests, DebugDrawingIsOffByDefault) {
        EXPECT_FALSE(renderer->IsDebugDrawEnabled());
    }

    TEST_F(DebugDrawTests, WithTheFlagTheColliderEdgesAreDrawn) {
        const Pixel center = DrawnCenter(true);

        EXPECT_EQ(center, Red);
    }

    TEST_F(DebugDrawTests, WithoutTheFlagNothingIsDrawn) {
        const Pixel center = DrawnCenter(false);

        EXPECT_NE(center, Red);
    }
}
