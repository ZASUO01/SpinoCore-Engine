// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <optional>
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

        void DrawUIPass();
    private:
        void BuildUIQuadVertices();

        std::optional<Graphics::Shaders::Shader> mUIShader{};
        std::optional<Graphics::Geometry::VertexArray> mUIQuadVertices{};
    };
}
