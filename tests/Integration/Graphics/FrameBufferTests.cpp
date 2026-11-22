// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <array>
#include <utility>
#include <gtest/gtest.h>
#include <glad/glad.h>
#include "GLTestContext.h"
#include "Graphics/FrameBuffer.h"
#include "Graphics/Resources/Texture.h"
#include "SpinoCore/Utils/Size2D.h"

namespace {
    using SpinoCore::Graphics::FrameBuffer;
    using SpinoCore::Graphics::FrameBufferFormat;
    using SpinoCore::Graphics::FrameBufferSpecification;
    using SpinoCore::Tests::GLTestContext;
    using SpinoCore::Utils::Size2D;

    class FrameBufferTests : public GLTestContext {
    protected:
        [[nodiscard]] static GLint BoundFrameBuffer() {
            GLint bound = 0;
            glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &bound);
            return bound;
        }

        [[nodiscard]] static std::array<GLint, 4> Viewport() {
            std::array<GLint, 4> viewport{};
            glGetIntegerv(GL_VIEWPORT, viewport.data());
            return viewport;
        }

        [[nodiscard]] static GLint DrawBufferOf(const FrameBuffer& frameBuffer) {
            GLint drawBuffer = 0;
            glGetNamedFramebufferParameteriv(frameBuffer.GetID(), GL_DRAW_BUFFER, &drawBuffer);
            return drawBuffer;
        }
    };

    TEST_F(FrameBufferTests, ADepthOnlyFrameBufferIsComplete) {
        const FrameBuffer shadowMap{{.width = 64, .height = 32, .format = FrameBufferFormat::DepthOnlyTexture}};

        EXPECT_EQ(glCheckNamedFramebufferStatus(shadowMap.GetID(), GL_FRAMEBUFFER), static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE));
    }

    TEST_F(FrameBufferTests, ADepthOnlyFrameBufferDrawsNoColor) {
        const FrameBuffer shadowMap{{.width = 64, .height = 32, .format = FrameBufferFormat::DepthOnlyTexture}};

        EXPECT_EQ(DrawBufferOf(shadowMap), GL_NONE);
    }

    TEST_F(FrameBufferTests, ADepthOnlyFrameBufferHasADepthTextureOfItsSize) {
        const FrameBuffer shadowMap{{.width = 64, .height = 32, .format = FrameBufferFormat::DepthOnlyTexture}};

        ASSERT_NE(shadowMap.GetTexture(), nullptr);
        EXPECT_EQ(shadowMap.GetTexture()->GetSize(), (Size2D{64, 32}));
    }

    TEST_F(FrameBufferTests, AColorFrameBufferIsComplete) {
        const FrameBuffer postProcess{{.width = 64, .height = 32, .format = FrameBufferFormat::ColorWithDepthRBO}};

        EXPECT_EQ(glCheckNamedFramebufferStatus(postProcess.GetID(), GL_FRAMEBUFFER), static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE));
    }

    TEST_F(FrameBufferTests, BindingCoversTheWholeFrameBuffer) {
        FrameBuffer shadowMap{{.width = 64, .height = 32, .format = FrameBufferFormat::DepthOnlyTexture}};

        shadowMap.Bind();

        EXPECT_EQ(Viewport(), (std::array<GLint, 4>{0, 0, 64, 32}));
        shadowMap.Unbind();
    }

    TEST_F(FrameBufferTests, UnbindingRestoresThePreviousFrameBuffer) {
        FrameBuffer shadowMap{{.width = 64, .height = 32, .format = FrameBufferFormat::DepthOnlyTexture}};
        const GLint previous = BoundFrameBuffer();
        shadowMap.Bind();

        shadowMap.Unbind();

        EXPECT_EQ(BoundFrameBuffer(), previous);
    }

    TEST_F(FrameBufferTests, UnbindingRestoresThePreviousViewport) {
        FrameBuffer shadowMap{{.width = 64, .height = 32, .format = FrameBufferFormat::DepthOnlyTexture}};
        const auto previous = Viewport();
        shadowMap.Bind();

        shadowMap.Unbind();

        EXPECT_EQ(Viewport(), previous);
    }

    TEST_F(FrameBufferTests, ResizingRecreatesTheAttachmentAtTheNewSize) {
        FrameBuffer postProcess{{.width = 64, .height = 32, .format = FrameBufferFormat::ColorWithDepthRBO}};

        postProcess.Resize(16, 8);

        EXPECT_EQ(postProcess.GetTexture()->GetSize(), (Size2D{16, 8}));
    }

    TEST_F(FrameBufferTests, MovingTransfersTheGLObjects) {
        FrameBuffer original{{.width = 64, .height = 32, .format = FrameBufferFormat::DepthOnlyTexture}};
        const auto id = original.GetID();

        const FrameBuffer moved{std::move(original)};

        EXPECT_EQ(moved.GetID(), id);
        EXPECT_EQ(original.GetID(), 0u);
    }

    TEST_F(FrameBufferTests, AFrameBufferWithoutSizeIsRejected) {
        EXPECT_ANY_THROW((FrameBuffer{{.width = 0, .height = 0, .format = FrameBufferFormat::DepthOnlyTexture}}));
    }
}
