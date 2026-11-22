// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>

namespace SpinoCore::Graphics::Shaders::Sources {
    namespace DebugShaderLocation {
        inline constexpr std::int32_t ViewProjection = 0;
    }

    inline constexpr auto DebugVertexShaderSource = R"(
        #version 460 core

        layout (location = 0) in vec3 inPosition;
        layout (location = 1) in vec4 inColor;
        layout (location = 0) uniform mat4 uViewProjection;

        out vec4 fragColor;

        void main() {
            fragColor = inColor;
            gl_Position = uViewProjection * vec4(inPosition, 1.0);
        }
    )";

    inline constexpr auto DebugFragmentShaderSource = R"(
        #version 460 core

        in vec4 fragColor;
        out vec4 outColor;

        void main() {
            outColor = fragColor;
        }
    )";
}
