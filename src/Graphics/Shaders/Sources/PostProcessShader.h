// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once

namespace SpinoCore::Graphics::Shaders::Sources {
    inline constexpr auto PostProcessVertexShaderSource = R"(
        #version 460 core

        out vec2 fragTextureCoord;

        void main() {
            vec2 corner = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
            fragTextureCoord = corner;
            gl_Position = vec4(corner * 2.0 - 1.0, 0.0, 1.0);
        }
    )";

    inline constexpr auto PostProcessFragmentHeaderSource = R"(
        #version 460 core

        in vec2 fragTextureCoord;
        out vec4 outColor;

        layout (location = 100, binding = 0) uniform sampler2D uScene;
        layout (location = 101) uniform vec2 uResolution;
        layout (location = 102) uniform float uTime;
    )";

    inline constexpr auto PassthroughFragmentSource = R"(
        void main() {
            outColor = texture(uScene, fragTextureCoord);
        }
    )";

    inline constexpr auto GrayscaleFragmentSource = R"(
        layout (location = 0) uniform float uStrength;

        void main() {
            vec4 color = texture(uScene, fragTextureCoord);
            float luminance = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
            outColor = vec4(mix(color.rgb, vec3(luminance), uStrength), color.a);
        }
    )";

    inline constexpr auto VignetteFragmentSource = R"(
        layout (location = 0) uniform float uRadius;
        layout (location = 1) uniform float uSoftness;

        void main() {
            vec4 color = texture(uScene, fragTextureCoord);
            vec2 centered = fragTextureCoord - 0.5;
            centered.x *= uResolution.x / max(uResolution.y, 1.0);
            float shade = smoothstep(uRadius, uRadius - uSoftness, length(centered));
            outColor = vec4(color.rgb * shade, color.a);
        }
    )";

    inline constexpr auto ExposureFragmentSource = R"(
        layout (location = 0) uniform float uExposure;
        layout (location = 1) uniform float uGamma;

        void main() {
            vec4 color = texture(uScene, fragTextureCoord);
            outColor = vec4(pow(color.rgb * uExposure, vec3(1.0 / max(uGamma, 0.001))), color.a);
        }
    )";
}
