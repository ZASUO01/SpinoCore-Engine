// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>

namespace SpinoCore::Graphics::Shaders::Sources {
    namespace UIShaderLocation {
        inline constexpr std::int32_t MVP = 0;
        inline constexpr std::int32_t TextureRect = 1;
        inline constexpr std::int32_t TextureMix = 2;
        inline constexpr std::int32_t Color = 3;
        inline constexpr std::int32_t Size = 4;
        inline constexpr std::int32_t Radius = 5;
        inline constexpr std::int32_t Opacity = 6;

    }

    inline constexpr auto UIVertexShaderSource = R"(
        #version 460 core

        layout (location = 0) in vec2 inPos;
        layout (location = 1) in vec2 inTextureCoord;

        layout (location = 0) uniform mat4 uMVP;
        layout (location = 1) uniform vec4 uTextureRect;

        out vec2 fragTextureCoord;
        out vec2 fragQuadCoord;

        void main() {
            gl_Position = uMVP * vec4(inPos, 0.0, 1.0);
            fragTextureCoord = uTextureRect.xy + inPos * uTextureRect.zw;
            fragQuadCoord = inPos;
        }
    )";

    inline constexpr auto UIFragmentShaderSource = R"(
        #version 460 core

        in vec2 fragTextureCoord;
        in vec2 fragQuadCoord;

        layout (binding = 0) uniform sampler2D uTexture;
        layout (location = 2) uniform float uTextureMix;
        layout (location = 3) uniform vec4 uColor;
        layout (location = 4) uniform vec2 uSize;
        layout (location = 5) uniform float uRadius;
        layout (location = 6) uniform float uOpacity;

        out vec4 outColor;

        float RoundedBoxSDF(vec2 p, vec2 halfSize, float radius) {
            vec2 q = abs(p) - halfSize + radius;
            return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - radius;
        }

        void main() {
            vec4 baseColor = uColor;

            // The texture is laid over the color ("over" compositing): where it is transparent the color shows through.
            if (uTextureMix > 0.0) {
                vec4 texColor = texture(uTexture, fragTextureCoord);
                float textureAlpha = texColor.a * uTextureMix;
                float alpha = textureAlpha + uColor.a * (1.0 - textureAlpha);
                vec3 rgb = texColor.rgb * textureAlpha + uColor.rgb * uColor.a * (1.0 - textureAlpha);
                baseColor = vec4(alpha > 0.0 ? rgb / alpha : vec3(0.0), alpha);
            }

            baseColor.a *= uOpacity;

            if (uRadius > 0.0) {
                vec2 halfSize = uSize * 0.5;
                float r = min(uRadius, min(halfSize.x, halfSize.y));
                float d = RoundedBoxSDF((fragQuadCoord - 0.5) * uSize, halfSize, r);
                baseColor.a *= 1.0 - smoothstep(-0.5, 0.5, d);
            }

            outColor = baseColor;
        }
    )";
}
