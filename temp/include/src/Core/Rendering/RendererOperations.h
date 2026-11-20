// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <optional>

#include "RenderCommands.h"
#include "Graphics/Geometry/VertexArray.h"
#include "Graphics/Shaders/Shader.h"

namespace SpinoCore::Core::Rendering {
    class RendererOperations {
    public:
        RendererOperations();
        ~RendererOperations();

        RendererOperations(const RendererOperations&) = delete;
        RendererOperations& operator=(const RendererOperations&) = delete;
        RendererOperations(RendererOperations&&) = delete;
        RendererOperations& operator=(RendererOperations&&) = delete;

        void DrawUIPass(UIDrawList& drawList, const Math::Matrix4f& orthoMatrix) const;
    private:
        void BuildUIQuadVertices();
        void BuildWhiteTexture();

        std::optional<Graphics::Shaders::Shader> mUIShader{};
        std::optional<Graphics::Geometry::VertexArray> mUIQuadVertices{};
        std::uint32_t mWhiteTexture{0}; // 1x1 white, used by untextured UI elements
    };
}
