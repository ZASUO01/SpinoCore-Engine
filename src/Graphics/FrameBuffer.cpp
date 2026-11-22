// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "FrameBuffer.h"
#include <exception>
#include <format>
#include <stdexcept>
#include <utility>
#include <glad/glad.h>
#include "Graphics/Resources/Texture.h"

namespace SpinoCore::Graphics {
    FrameBuffer::FrameBuffer(const FrameBufferSpecification &spec)
    :mSpec(spec) {
        Reset();
    }

    FrameBuffer::~FrameBuffer() {
        Release();
    }

    FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
    :mFBO(std::exchange(other.mFBO, 0))
    ,mRBO(std::exchange(other.mRBO, 0))
    ,mSpec(other.mSpec)
    ,mTexture(std::move(other.mTexture))
    {}

    FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept {
        if (this != &other) {
            Release();
            mFBO = std::exchange(other.mFBO, 0);
            mRBO = std::exchange(other.mRBO, 0);
            mSpec = other.mSpec;
            mTexture = std::move(other.mTexture);
        }
        return *this;
    }

    void FrameBuffer::Bind() {
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &mPreviousFBO);
        glGetIntegerv(GL_VIEWPORT, mPreviousViewport.data());

        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
        glViewport(0, 0, mSpec.width, mSpec.height);
    }

    void FrameBuffer::Unbind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(mPreviousFBO));
        glViewport(mPreviousViewport[0], mPreviousViewport[1], mPreviousViewport[2], mPreviousViewport[3]);
    }

    void FrameBuffer::Resize(const int width, const int height) {
        if (width == mSpec.width && height == mSpec.height) return;

        mSpec.width = width;
        mSpec.height = height;
        Reset();
    }

    Resources::Texture *FrameBuffer::GetTexture() const {
        return mTexture.get();
    }

    void FrameBuffer::Reset() {
        Release();

        try {
            glCreateFramebuffers(1, &mFBO);

            if (mSpec.format == FrameBufferFormat::ColorWithDepthRBO) {
                mTexture = std::make_unique<Resources::Texture>(mSpec.width, mSpec.height, Resources::CreationType::Empty);
                glNamedFramebufferTexture(mFBO, GL_COLOR_ATTACHMENT0, mTexture->GetID(), 0);

                glCreateRenderbuffers(1, &mRBO);
                glNamedRenderbufferStorage(mRBO, GL_DEPTH24_STENCIL8, mSpec.width, mSpec.height);
                glNamedFramebufferRenderbuffer(mFBO, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRBO);
            } else {
                mTexture = std::make_unique<Resources::Texture>(mSpec.width, mSpec.height, Resources::CreationType::Depth);
                glNamedFramebufferTexture(mFBO, GL_DEPTH_ATTACHMENT, mTexture->GetID(), 0);
                glNamedFramebufferDrawBuffer(mFBO, GL_NONE);
                glNamedFramebufferReadBuffer(mFBO, GL_NONE);
            }

            if (const GLenum status = glCheckNamedFramebufferStatus(mFBO, GL_FRAMEBUFFER); status != GL_FRAMEBUFFER_COMPLETE) {
                throw std::runtime_error(std::format("[FRAME BUFFER] Incomplete frame buffer, GL status {:#x}.", status));
            }
        } catch (const std::exception&) {
            Release();
            throw;
        }
    }

    void FrameBuffer::Release() noexcept {
        if (mFBO != 0) {
            glDeleteFramebuffers(1, &mFBO);
            mFBO = 0;
        }
        if (mRBO != 0) {
            glDeleteRenderbuffers(1, &mRBO);
            mRBO = 0;
        }
        mTexture.reset();
    }
}
