// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <array>
#include <cstdint>
#include <memory>

namespace SpinoCore::Graphics {
    namespace Resources {
        class Texture;
    }

    enum class FrameBufferFormat {
        ColorWithDepthRBO,
        DepthOnlyTexture
    };

    struct FrameBufferSpecification {
        int width;
        int height;
        FrameBufferFormat format;
    };

    class FrameBuffer final {
    public:
        explicit FrameBuffer(const FrameBufferSpecification& spec);
        ~FrameBuffer();

        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer& operator=(const FrameBuffer&) = delete;
        FrameBuffer(FrameBuffer&& other) noexcept;
        FrameBuffer& operator=(FrameBuffer&& other) noexcept;

        void Bind();
        void Unbind() const;
        void Resize(int width, int height);

        [[nodiscard]] Resources::Texture* GetTexture() const;
        [[nodiscard]] std::uint32_t GetID() const noexcept { return mFBO; }

    private:
        void Reset();
        void Release() noexcept;

        std::uint32_t mFBO{0};
        std::uint32_t mRBO{0};

        FrameBufferSpecification mSpec;
        std::unique_ptr<Resources::Texture> mTexture;

        std::int32_t mPreviousFBO{0};
        std::array<std::int32_t, 4> mPreviousViewport{};
    };
}
