// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>

namespace SpinoCore::Graphics::Shaders {
    // Uniform locations of the UI shader (match the layout(location = N) below).
    namespace UIShaderLocation {
        inline constexpr std::int32_t Ortho = 0;
        inline constexpr std::int32_t Transform = 1;
        inline constexpr std::int32_t TextureRect = 2;
        inline constexpr std::int32_t Color = 3;
    }

    // Vertex input: unit quad (0..1) at attribute location 0, also used as texture coordinate.
    // Texture unit 0: bind a 1x1 white texture for untextured elements.
    inline constexpr auto UIVertexShaderSource = R"(
        #version 460 core

        layout (location = 0) in vec2 inPos;

        layout (location = 0) uniform mat4 uOrtho;
        layout (location = 1) uniform mat4 uTransform;
        layout (location = 2) uniform vec4 uTextureRect;

        out vec2 fragTextureCoord;

        void main() {
            gl_Position = uOrtho * uTransform * vec4(inPos, 0.0, 1.0);
            fragTextureCoord = uTextureRect.xy + inPos * uTextureRect.zw;
        }
    )";

    inline constexpr auto UIFragmentShaderSource = R"(
        #version 460 core

        in vec2 fragTextureCoord;

        layout (binding = 0) uniform sampler2D uTexture;
        layout (location = 3) uniform vec4 uColor;

        out vec4 outColor;

        void main() {
            outColor = uColor * texture(uTexture, fragTextureCoord);
        }
    )";
}
