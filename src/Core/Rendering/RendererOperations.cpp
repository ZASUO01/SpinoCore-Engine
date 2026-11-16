// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "RendererOperations.h"
#include <cstdint>
#include <span>
#include <glad/glad.h>
#include "Graphics/Shaders/UIShader.h"

namespace SpinoCore::Core::Rendering {
    RendererOperations::RendererOperations(){
        mUIShader.emplace(Graphics::Shaders::UIVertexShaderSource, Graphics::Shaders::UIFragmentShaderSource);

        BuildUIQuadVertices();
    }

    RendererOperations::~RendererOperations() = default;

    void RendererOperations::DrawUIPass() {

    }

    void RendererOperations::BuildUIQuadVertices() {
        static constexpr float vertices[] = {
            0.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,
            1.0f, 0.0f,
        };

        static constexpr std::uint32_t indices[] = {
            0, 1, 2,
            2, 3, 0,
        };

        mUIQuadVertices.emplace(Graphics::Geometry::VertexArray::Input{
            .vertexData = std::as_bytes(std::span(vertices)),
            .indices = indices,
            .layout = {{.location = 0, .type = Graphics::Geometry::AttributeType::Float2}},
        });
    }
}
