// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "VertexArray.h"
#include <algorithm>
#include <format>
#include <stdexcept>
#include <glad/glad.h>

namespace SpinoCore::Graphics::Geometry {
    namespace {
        constexpr GLuint BindingIndex = 0;

        struct AttributeFormat final {
            GLint components;
            GLenum type;
            GLboolean normalized;
            bool integer;
            GLuint byteSize;
        };

        [[nodiscard]] constexpr AttributeFormat FormatOf(const AttributeType type) noexcept {
            switch (type) {
                case AttributeType::Float: return {1, GL_FLOAT, GL_FALSE, false, 4};
                case AttributeType::Float2: return {2, GL_FLOAT, GL_FALSE, false, 8};
                case AttributeType::Float3: return {3, GL_FLOAT, GL_FALSE, false, 12};
                case AttributeType::Float4: return {4, GL_FLOAT, GL_FALSE, false, 16};
                case AttributeType::UByte4Norm: return {4, GL_UNSIGNED_BYTE, GL_TRUE, false, 4};
                case AttributeType::UShort2Norm: return {2, GL_UNSIGNED_SHORT, GL_TRUE, false, 4};
                case AttributeType::UByte4: return {4, GL_UNSIGNED_BYTE, GL_FALSE, true, 4};
                case AttributeType::UInt: return {1, GL_UNSIGNED_INT, GL_FALSE, true, 4};
            }
            return {1, GL_FLOAT, GL_FALSE, false, 4};
        }

        [[nodiscard]] GLuint StrideOf(const std::vector<VertexAttribute>& layout) noexcept {
            GLuint stride = 0;
            for (const auto&[location, type] : layout) stride += FormatOf(type).byteSize;
            return stride;
        }

        void Validate(const VertexArray::Input& input, const GLuint stride) {
            if (input.layout.empty()) {
                throw std::invalid_argument("[VERTEX ARRAY] Vertex layout must not be empty.");
            }
            if (input.vertexData.empty()) {
                throw std::invalid_argument("[VERTEX ARRAY] Vertex data must not be empty.");
            }
            if (input.vertexData.size() % stride != 0) {
                throw std::invalid_argument(std::format(
                    "[VERTEX ARRAY] Vertex data size {} is not a multiple of the vertex stride {}.",
                    input.vertexData.size(), stride));
            }

            GLint maxAttributes = 0;
            glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttributes);

            std::vector<std::uint32_t> locations;
            for (const auto&[location, type] : input.layout) {
                if (location >= static_cast<std::uint32_t>(maxAttributes)) {
                    throw std::invalid_argument(std::format(
                        "[VERTEX ARRAY] Attribute location {} exceeds the limit of {}.", location, maxAttributes));
                }
                if (std::ranges::find(locations, location) != locations.end()) {
                    throw std::invalid_argument(std::format(
                        "[VERTEX ARRAY] Attribute location {} is used more than once.", location));
                }
                locations.push_back(location);
            }

            const std::size_t vertexCount = input.vertexData.size() / stride;
            if (const auto highest = std::ranges::max_element(input.indices);
                highest != input.indices.end() && *highest >= vertexCount) {
                throw std::invalid_argument(std::format(
                    "[VERTEX ARRAY] Index {} is out of range for {} vertices.", *highest, vertexCount));
            }
        }
    }

    VertexArray::VertexArray(const Input& input) {
        const GLuint stride = StrideOf(input.layout);
        Validate(input, stride);

        mVertexCount = static_cast<std::uint32_t>(input.vertexData.size() / stride);
        mIndexCount = static_cast<std::uint32_t>(input.indices.size());

        glCreateBuffers(1, &mVertexBuffer);
        glNamedBufferStorage(mVertexBuffer, static_cast<GLsizeiptr>(input.vertexData.size()), input.vertexData.data(), 0);

        glCreateVertexArrays(1, &mVertexArray);
        glVertexArrayVertexBuffer(mVertexArray, BindingIndex, mVertexBuffer, 0, static_cast<GLsizei>(stride));

        GLuint offset = 0;
        for (const auto&[location, type] : input.layout) {
            const AttributeFormat format = FormatOf(type); // NOLINT

            glEnableVertexArrayAttrib(mVertexArray, location);
            if (format.integer) {
                glVertexArrayAttribIFormat(mVertexArray, location, format.components, format.type, offset);
            } else {
                glVertexArrayAttribFormat(mVertexArray, location, format.components, format.type,
                    format.normalized, offset);
            }
            glVertexArrayAttribBinding(mVertexArray, location, BindingIndex);

            offset += format.byteSize;
        }

        if (!input.indices.empty()) {
            glCreateBuffers(1, &mIndexBuffer);
            glNamedBufferStorage(mIndexBuffer, static_cast<GLsizeiptr>(input.indices.size_bytes()), input.indices.data(), 0);
            glVertexArrayElementBuffer(mVertexArray, mIndexBuffer);
        }
    }

    VertexArray::~VertexArray() {
        if (mVertexArray != 0) glDeleteVertexArrays(1, &mVertexArray);
        if (mIndexBuffer != 0) glDeleteBuffers(1, &mIndexBuffer);
        if (mVertexBuffer != 0) glDeleteBuffers(1, &mVertexBuffer);
    }

    void VertexArray::Draw() const noexcept {
        glBindVertexArray(mVertexArray);
        if (mIndexCount > 0) {
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mIndexCount), GL_UNSIGNED_INT, nullptr);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(mVertexCount));
        }
    }
}