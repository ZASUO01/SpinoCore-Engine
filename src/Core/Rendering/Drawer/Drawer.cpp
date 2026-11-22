// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Drawer.h"
#include <algorithm>
#include <cstdint>
#include <span>
#include <glad/glad.h>
#include "Graphics/Shaders/Sources/UIShader.h"

namespace SpinoCore::Core::Rendering {
    Drawer::Drawer() {
        using namespace Graphics::Shaders;

        mUIShader.emplace(Sources::UIVertexShaderSource, Sources::UIFragmentShaderSource);

        BuildUIQuadVertices();
    }

    void Drawer::DrawUI(const Math::Matrix4f& ortho) {
        using namespace Graphics::Shaders::Sources;

        if (mUIDrawList.commands.empty()) return;

        std::ranges::stable_sort(mUIDrawList.commands, [](const auto& a, const auto& b) {
            return a.key < b.key;
        });

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        mUIShader->SetActive();
        std::uint32_t boundTexture = 0;
        for (auto& command : mUIDrawList.commands) {
            const Math::Matrix4f MVP = ortho * command.transform;

            mUIShader->SetUniform(UIShaderLocation::MVP, MVP);
            mUIShader->SetUniform(UIShaderLocation::TextureRect, command.textureRect);
            mUIShader->SetUniform(UIShaderLocation::Color, command.color);
            mUIShader->SetUniform(UIShaderLocation::Size, command.size);
            mUIShader->SetUniform(UIShaderLocation::Radius, command.borderRadius);
            mUIShader->SetUniform(UIShaderLocation::BorderWidth, command.borderWidth);
            mUIShader->SetUniform(UIShaderLocation::BorderColor, command.borderColor);
            mUIShader->SetUniform(UIShaderLocation::Opacity, command.opacity);

            const bool textured = command.textureId != 0 && command.textureMix > 0.0f;
            mUIShader->SetUniform(UIShaderLocation::TextureMix, textured ? command.textureMix : 0.0f);
            mUIShader->SetUniform(UIShaderLocation::TintTexture, textured && command.tintTexture ? 1 : 0);
           
            if (textured && command.textureId != boundTexture) {
                glBindTextureUnit(0, command.textureId);
                boundTexture = command.textureId;
            }

            mUIQuadVertices->Draw();
        }

        mUIDrawList.commands.clear();

        glBindTextureUnit(0, 0);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }

    void Drawer::BuildUIQuadVertices() {
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
