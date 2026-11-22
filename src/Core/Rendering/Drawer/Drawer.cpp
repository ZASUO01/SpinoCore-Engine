// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Drawer.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <variant>
#include <glad/glad.h>
#include "Graphics/Geometry/VertexArray.h"
#include "Graphics/Resources/Model.h"
#include "Graphics/Shaders/Sources/ModelShader.h"
#include "Graphics/Shaders/Sources/UIShader.h"
#include "SpinoCore/Core/Lighting/GlobalLight.h"
#include "SpinoCore/Graphics/PostProcess.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::Rendering {
    namespace {
        constexpr Math::Vector3f LightDirection{-0.4f, -1.0f, -0.3f};
    }

    Drawer::Drawer() {
        using namespace Graphics::Shaders;

        mUIShader.emplace(Sources::UIVertexShaderSource, Sources::UIFragmentShaderSource);
        mModelShader.emplace(std::string(Sources::ModelSkinningPreludeSource) + Sources::ModelVertexShaderBody, Sources::ModelFragmentShaderSource);
        mShadowShader.emplace(std::string(Sources::ModelSkinningPreludeSource) + Sources::ShadowVertexShaderBody, Sources::ShadowFragmentShaderSource);
        mWhiteTexture.emplace();

        BuildUIQuadVertices();
        glCreateVertexArrays(1, &mFullScreenVertexArray);
    }

    Drawer::~Drawer() {
        if (mFullScreenVertexArray != 0) glDeleteVertexArrays(1, &mFullScreenVertexArray);
    }

    void Drawer::DrawPostProcess(const Graphics::Shaders::Shader& shader, const std::uint32_t sceneTexture, const Math::Vector2f& resolution,
                                 const float time, const std::span<const Graphics::PostProcessUniform> uniforms) const {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        shader.SetActive();
        for (std::size_t index = 0; index < uniforms.size(); ++index) {
            const auto location = static_cast<std::int32_t>(index);
            std::visit([&shader, location](const auto& value) { shader.SetUniform(location, value); }, uniforms[index].value);
        }
        shader.SetUniform(Graphics::PostProcessLocation::Resolution, resolution);
        shader.SetUniform(Graphics::PostProcessLocation::Time, time);

        glBindTextureUnit(0, sceneTexture);
        glBindVertexArray(mFullScreenVertexArray);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        glBindTextureUnit(0, 0);
        glEnable(GL_DEPTH_TEST);
    }

    void Drawer::DrawShadows(const Math::Matrix4f& lightViewProjection) {
        using namespace Graphics::Shaders::Sources;

        if (mModelDrawList.commands.empty()) return;

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDisable(GL_BLEND);

        mShadowShader->SetActive();
        mShadowShader->SetUniform(ModelShaderLocation::ViewProjection, lightViewProjection);
        for (const auto& command : mModelDrawList.commands) DrawModelMeshes(*mShadowShader, command, false);
    }

    void Drawer::DrawModels(const Math::Matrix4f& viewProjection, const ModelLighting* lighting) {
        using namespace Graphics::Shaders::Sources;

        if (mModelDrawList.commands.empty()) return;

        std::ranges::stable_sort(mModelDrawList.commands, [](const auto& a, const auto& b) {
            return a.key < b.key;
        });

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDisable(GL_BLEND);

        mModelShader->SetActive();
        mModelShader->SetUniform(ModelShaderLocation::ViewProjection, viewProjection);
        if (lighting) {
            const auto& settings = lighting->light.GetSettings();
            mModelShader->SetUniform(ModelShaderLocation::GlobalLight, 1);
            mModelShader->SetUniform(ModelShaderLocation::LightDirection, lighting->light.GetDirection());
            mModelShader->SetUniform(ModelShaderLocation::LightColor, settings.color * settings.intensity);
            mModelShader->SetUniform(ModelShaderLocation::AmbientColor, settings.ambientColor);
            mModelShader->SetUniform(ModelShaderLocation::LightViewProjection, lighting->light.GetViewProjection());
            glBindTextureUnit(ModelShaderTextureUnit::ShadowMap, lighting->shadowMap);
        } else {
            mModelShader->SetUniform(ModelShaderLocation::GlobalLight, 0);
            mModelShader->SetUniform(ModelShaderLocation::LightDirection, LightDirection);
        }

        for (const auto& command : mModelDrawList.commands) DrawModelMeshes(*mModelShader, command, true);

        mModelDrawList.commands.clear();
        glBindTextureUnit(ModelShaderTextureUnit::Albedo, 0);
        glBindTextureUnit(ModelShaderTextureUnit::ShadowMap, 0);
    }

    void Drawer::DrawModelMeshes(const Graphics::Shaders::Shader& shader, const ModelDrawCommand& command, const bool withMaterials) const {
        using namespace Graphics::Shaders::Sources;

        if (!command.model) return;

        const bool posed = !command.bones.empty() && command.bones.size() == command.model->GetSkeleton().bones.size();
        shader.SetUniform(ModelShaderLocation::Model, command.world);
        if (posed) shader.SetUniform(ModelShaderLocation::Bones, command.bones);

        for (const auto&[vertices, materialIdx, skinned] : command.model->GetMeshes()) {
            shader.SetUniform(ModelShaderLocation::Skinned, skinned && posed ? 1 : 0);
            if (withMaterials) {
                const auto&[baseColor, textureId] = command.model->GetMaterials()[materialIdx];
                shader.SetUniform(ModelShaderLocation::BaseColor, baseColor);
                BindModelTexture(textureId);
            }
            vertices->Draw();
        }
    }

    void Drawer::BindModelTexture(const std::string& textureId) const {
        const auto* textures = mModelDrawList.textures;
        if (textureId.empty() || !textures || !textures->Contains(textureId)) {
            glBindTextureUnit(Graphics::Shaders::Sources::ModelShaderTextureUnit::Albedo, mWhiteTexture->GetID());
            return;
        }
        glBindTextureUnit(Graphics::Shaders::Sources::ModelShaderTextureUnit::Albedo, textures->Get(textureId).GetID());
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
