// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>
#include <gtest/gtest.h>
#include <glad/glad.h>
#include "GLTestContext.h"
#include "Graphics/Geometry/VertexArray.h"
#include "Graphics/Shaders/Shader.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Graphics::Geometry::AttributeType;
    using SpinoCore::Graphics::Geometry::VertexArray;
    using SpinoCore::Graphics::Geometry::VertexAttribute;
    using SpinoCore::Graphics::Shaders::Shader;
    using SpinoCore::Math::Vector4f;
    using SpinoCore::Tests::GLTestContext;
    using SpinoCore::Tests::Pixel;

    constexpr auto PositionOnlyVertexSource = R"(
        #version 460 core
        layout (location = 0) in vec2 inPos;
        void main() { gl_Position = vec4(inPos, 0.0, 1.0); }
    )";

    constexpr auto UniformColorFragmentSource = R"(
        #version 460 core
        layout (location = 1) uniform vec4 uColor;
        out vec4 outColor;
        void main() { outColor = uColor; }
    )";

    constexpr auto VertexColorVertexSource = R"(
        #version 460 core
        layout (location = 0) in vec2 inPos;
        layout (location = 3) in vec4 inColor;
        out vec4 color;
        void main() { gl_Position = vec4(inPos, 0.0, 1.0); color = inColor; }
    )";

    constexpr auto VertexColorFragmentSource = R"(
        #version 460 core
        in vec4 color;
        out vec4 outColor;
        void main() { outColor = color; }
    )";

    constexpr auto IntegerIdVertexSource = R"(
        #version 460 core
        layout (location = 0) in vec2 inPos;
        layout (location = 1) in uint inId;
        flat out uint id;
        void main() { gl_Position = vec4(inPos, 0.0, 1.0); id = inId; }
    )";

    constexpr auto IntegerIdFragmentSource = R"(
        #version 460 core
        flat in uint id;
        out vec4 outColor;
        void main() { outColor = id == 123456u ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0); }
    )";

    constexpr Pixel Transparent{0, 0, 0, 0};
    constexpr Pixel White{255, 255, 255, 255};
    constexpr Pixel Green{0, 255, 0, 255};

    struct Position final {
        float x;
        float y;
    };

    constexpr std::array FullScreenCorners{Position{-1, -1}, Position{1, -1}, Position{1, 1}, Position{-1, 1}};
    constexpr std::array<std::uint32_t, 6> FullScreenIndices{0, 1, 2, 2, 3, 0};
    const std::vector<VertexAttribute> PositionLayout{{.location = 0, .type = AttributeType::Float2}};

    class VertexArrayTests : public GLTestContext {
    protected:
        void DrawWhite(const VertexArray& vertexArray) {
            const Shader shader{PositionOnlyVertexSource, UniformColorFragmentSource};
            shader.SetUniform(1, Vector4f{1.0f, 1.0f, 1.0f, 1.0f});
            shader.SetActive();
            vertexArray.Draw();
            glUseProgram(0);
        }
    };

    TEST_F(VertexArrayTests, ReportsHowManyVerticesAndIndicesItHolds) {
        const VertexArray quad{{
            .vertexData = std::as_bytes(std::span{FullScreenCorners}),
            .indices = FullScreenIndices,
            .layout = PositionLayout
        }};

        EXPECT_EQ(quad.GetVertexCount(), 4u);
        EXPECT_EQ(quad.GetIndexCount(), 6u);
    }

    TEST_F(VertexArrayTests, DrawsTheVerticesInOrderWhenThereAreNoIndices) {

        const std::array triangle{Position{-1, -1}, Position{1, -1}, Position{-1, 1}};
        const VertexArray vertexArray{{.vertexData = std::as_bytes(std::span{triangle}), .indices = {}, .layout = PositionLayout}};

        DrawWhite(vertexArray);

        EXPECT_EQ(vertexArray.GetIndexCount(), 0u);
        EXPECT_EQ(ReadPixel(1, 1), White);
        EXPECT_EQ(ReadPixel(TargetSize - 2, TargetSize - 2), Transparent);
    }

    TEST_F(VertexArrayTests, DrawsThroughTheIndicesWhenTheyAreGiven) {
        const VertexArray quad{{
            .vertexData = std::as_bytes(std::span{FullScreenCorners}),
            .indices = FullScreenIndices,
            .layout = PositionLayout
        }};

        DrawWhite(quad);

        EXPECT_EQ(ReadPixel(1, 1), White);
        EXPECT_EQ(ReadPixel(TargetSize - 2, TargetSize - 2), White);
        EXPECT_EQ(ReadPixel(1, TargetSize - 2), White);
        EXPECT_EQ(ReadPixel(TargetSize - 2, 1), White);
    }

    TEST_F(VertexArrayTests, InterleavedAttributesUseTheRightOffsetsAndNormalizeBytes) {
        struct ColoredVertex final {
            Position position;
            std::array<std::uint8_t, 4> color;
        };
        std::array<ColoredVertex, 4> vertices{};
        for (std::size_t i = 0; i < vertices.size(); ++i) {
            vertices[i] = {FullScreenCorners[i], {0, 255, 0, 255}};
        }
        const VertexArray quad{{
            .vertexData = std::as_bytes(std::span{vertices}),
            .indices = FullScreenIndices,
            .layout = {{.location = 0, .type = AttributeType::Float2}, {.location = 3, .type = AttributeType::UByte4Norm}}
        }};
        const Shader shader{VertexColorVertexSource, VertexColorFragmentSource};

        shader.SetActive();
        quad.Draw();
        glUseProgram(0);

        EXPECT_EQ(ReadPixel(TargetSize / 2, TargetSize / 2), Green);
    }

    TEST_F(VertexArrayTests, IntegerAttributesReachTheShaderAsIntegers) {
        struct IdVertex final {
            Position position;
            std::uint32_t id;
        };
        std::array<IdVertex, 4> vertices{};
        for (std::size_t i = 0; i < vertices.size(); ++i) {
            vertices[i] = {FullScreenCorners[i], 123456u};
        }
        const VertexArray quad{{
            .vertexData = std::as_bytes(std::span{vertices}),
            .indices = FullScreenIndices,
            .layout = {{.location = 0, .type = AttributeType::Float2}, {.location = 1, .type = AttributeType::UInt}}
        }};
        const Shader shader{IntegerIdVertexSource, IntegerIdFragmentSource};

        shader.SetActive();
        quad.Draw();
        glUseProgram(0);

        EXPECT_EQ(ReadPixel(TargetSize / 2, TargetSize / 2), Green);
    }

    TEST_F(VertexArrayTests, RejectsAnEmptyLayout) {
        EXPECT_THROW(VertexArray({.vertexData = std::as_bytes(std::span{FullScreenCorners}), .indices = {}, .layout = {}}),
                     std::invalid_argument);
    }

    TEST_F(VertexArrayTests, RejectsEmptyVertexData) {
        EXPECT_THROW(VertexArray({.vertexData = {}, .indices = {}, .layout = PositionLayout}), std::invalid_argument);
    }

    TEST_F(VertexArrayTests, RejectsDataThatIsNotAWholeNumberOfVertices) {
        const auto bytes = std::as_bytes(std::span{FullScreenCorners});

        EXPECT_THROW(VertexArray({.vertexData = bytes.first(bytes.size() - 1), .indices = {}, .layout = PositionLayout}),
                     std::invalid_argument);
    }

    TEST_F(VertexArrayTests, RejectsTheSameLocationTwice) {
        const std::array<float, 4> data{};

        EXPECT_THROW(VertexArray({
            .vertexData = std::as_bytes(std::span{data}),
            .indices = {},
            .layout = {{.location = 0, .type = AttributeType::Float}, {.location = 0, .type = AttributeType::Float}}
        }), std::invalid_argument);
    }

    TEST_F(VertexArrayTests, RejectsALocationBeyondWhatTheDriverSupports) {
        GLint maxAttributes = 0;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttributes);
        const std::array<float, 2> data{};

        EXPECT_THROW(VertexArray({
            .vertexData = std::as_bytes(std::span{data}),
            .indices = {},
            .layout = {{.location = static_cast<std::uint32_t>(maxAttributes), .type = AttributeType::Float2}}
        }), std::invalid_argument);
    }

    TEST_F(VertexArrayTests, RejectsAnIndexPastTheLastVertex) {
        constexpr std::array<std::uint32_t, 3> indices{0, 1, 4};

        EXPECT_THROW(VertexArray({.vertexData = std::as_bytes(std::span{FullScreenCorners}), .indices = indices, .layout = PositionLayout}),
                     std::invalid_argument);
    }
}
