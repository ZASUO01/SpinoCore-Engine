// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <optional>
#include <span>
#include <string>

#include "DrawCommands.h"
#include "Graphics/Geometry/VertexArray.h"
#include "Graphics/Resources/Texture.h"
#include "Graphics/Shaders/Shader.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Graphics {
    struct PostProcessUniform;
}

namespace SpinoCore::Core::Lighting {
    class GlobalLight;
}

namespace SpinoCore::Core::Rendering {
    struct ModelLighting final {
        const Lighting::GlobalLight& light;
        std::uint32_t shadowMap{0};
    };

    class Drawer final {
    public:
        Drawer();
        ~Drawer();

        Drawer(const Drawer&) = delete;
        Drawer& operator=(const Drawer&) = delete;
        Drawer(Drawer&&) = delete;
        Drawer& operator=(Drawer&&) = delete;

        [[nodiscard]] ModelDrawList& GetModelDrawList() { return mModelDrawList;}
        [[nodiscard]] UIDrawList& GetUIDrawList() { return mUIDrawList; }

        void DrawShadows(const Math::Matrix4f& lightViewProjection);
        void DrawModels(const Math::Matrix4f& viewProjection, const ModelLighting* lighting = nullptr);
        void DrawUI(const Math::Matrix4f& ortho);
        void DrawPostProcess(const Graphics::Shaders::Shader& shader, std::uint32_t sceneTexture, const Math::Vector2f& resolution,
                             float time, std::span<const Graphics::PostProcessUniform> uniforms) const;
    private:
        ModelDrawList mModelDrawList;
        UIDrawList mUIDrawList;

        void BuildUIQuadVertices();

        void BindModelTexture(const std::string& textureId) const;
        void DrawModelMeshes(const Graphics::Shaders::Shader& shader, const ModelDrawCommand& command, bool withMaterials) const;

        std::optional<Graphics::Shaders::Shader> mUIShader{};
        std::optional<Graphics::Shaders::Shader> mModelShader{};
        std::optional<Graphics::Shaders::Shader> mShadowShader{};
        std::optional<Graphics::Resources::Texture> mWhiteTexture{};
        std::optional<Graphics::Geometry::VertexArray> mUIQuadVertices{};
        std::uint32_t mFullScreenVertexArray{0};
    };
}
