// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace SpinoCore::Graphics::Geometry {
    enum class AttributeType {
        Float,
        Float2,
        Float3,
        Float4,
        UByte4Norm,
        UShort2Norm,
        UByte4,
        UInt
    };

    struct VertexAttribute final {
        std::uint32_t location;
        AttributeType type;
    };

    class VertexArray final {
    public:
        struct Input final {
            std::span<const std::byte> vertexData;
            std::span<const std::uint32_t> indices;
            std::vector<VertexAttribute> layout;
        };

        explicit VertexArray(const Input& input);
        ~VertexArray();

        VertexArray(const VertexArray&) = delete;
        VertexArray& operator=(const VertexArray&) = delete;
        VertexArray(VertexArray&&) = delete;
        VertexArray& operator=(VertexArray&&) = delete;

        void Draw() const noexcept;

        [[nodiscard]] std::uint32_t GetVertexCount() const noexcept { return mVertexCount; }
        [[nodiscard]] std::uint32_t GetIndexCount() const noexcept { return mIndexCount; }

    private:
        std::uint32_t mVertexArray{0};
        std::uint32_t mVertexBuffer{0};
        std::uint32_t mIndexBuffer{0};
        std::uint32_t mVertexCount{0};
        std::uint32_t mIndexCount{0};
    };
}