// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <optional>

#include "DrawCommands.h"
#include "Graphics/Geometry/VertexArray.h"
#include "Graphics/Shaders/Shader.h"
#include "SpinoCore/Math/Matrix.h"

namespace SpinoCore::Core::Rendering {
    class Drawer final {
    public:
        Drawer();
        ~Drawer() = default;

        [[nodiscard]] UIDrawList& GetUIDrawList() { return mUIDrawList; }

        void DrawUI(const Math::Matrix4f& ortho);
    private:
        UIDrawList mUIDrawList;

        void BuildUIQuadVertices();

        std::optional<Graphics::Shaders::Shader> mUIShader{};
        std::optional<Graphics::Geometry::VertexArray> mUIQuadVertices{};
    };
}
