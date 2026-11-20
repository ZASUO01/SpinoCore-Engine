// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "RendererOperations.h"
#include <algorithm>
#include <cstdint>
#include <span>
#include <glad/glad.h>
#include "../../Graphics/Shaders/Sources/UIShader.h"

namespace SpinoCore::Core::Rendering {
    RendererOperations::RendererOperations(){
        mUIShader.emplace(Graphics::Shaders::UIVertexShaderSource, Graphics::Shaders::UIFragmentShaderSource);

        BuildUIQuadVertices();
        BuildWhiteTexture();
    }

    RendererOperations::~RendererOperations() {
        glDeleteTextures(1, &mWhiteTexture);
    }

    void RendererOperations::DrawUIPass(UIDrawList& drawList, const Math::Matrix4f& orthoMatrix) const {
        using namespace Graphics::Shaders;
        if (drawList.empty() || !mUIShader || !mUIQuadVertices) return;

        // Stable: equal draw orders keep the order the elements were collected in.
        std::ranges::stable_sort(drawList, {}, &UICommand::drawOrder);

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        mUIShader->SetActive();
        mUIShader->SetUniform(UIShaderLocation::Ortho, orthoMatrix);
        glBindTextureUnit(0, mWhiteTexture);

        for (const auto& cmd : drawList) {
            mUIShader->SetUniform(UIShaderLocation::Transform, cmd.transform);
            mUIShader->SetUniform(UIShaderLocation::TextureRect, cmd.textureRect);
            mUIShader->SetUniform(UIShaderLocation::Color, cmd.color);
            mUIShader->SetUniform(4, cmd.size);
            mUIShader->SetUniform(5, cmd.borderRadius);

            mUIQuadVertices->Draw();
        }

        drawList.clear();
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

    void RendererOperations::BuildWhiteTexture() {
        static constexpr std::uint8_t white[] = {255, 255, 255, 255};

        glCreateTextures(GL_TEXTURE_2D, 1, &mWhiteTexture);
        glTextureStorage2D(mWhiteTexture, 1, GL_RGBA8, 1, 1);
        glTextureSubImage2D(mWhiteTexture, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, white);
        glTextureParameteri(mWhiteTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(mWhiteTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
}
