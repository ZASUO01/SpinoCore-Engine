// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>

namespace SpinoCore::Graphics::Shaders::Sources {
    namespace ParticleShaderLocation {
        inline constexpr std::int32_t ViewProjection = 0;
        inline constexpr std::int32_t CameraRight = 1;
        inline constexpr std::int32_t CameraUp = 2;
    }

    inline constexpr auto ParticleVertexShaderSource = R"(
        #version 460 core

        layout (location = 0) in vec2 inCorner;
        layout (location = 1) in vec3 inPosition;
        layout (location = 2) in float inSize;
        layout (location = 3) in vec4 inColor;
        layout (location = 4) in float inRotation;

        layout (location = 0) uniform mat4 uViewProjection;
        layout (location = 1) uniform vec3 uCameraRight;
        layout (location = 2) uniform vec3 uCameraUp;

        out vec4 fragColor;
        out vec2 fragCorner;

        void main() {
            float c = cos(inRotation);
            float s = sin(inRotation);
            vec2 offset = vec2(c * inCorner.x - s * inCorner.y, s * inCorner.x + c * inCorner.y) * inSize;
            vec3 world = inPosition + uCameraRight * offset.x + uCameraUp * offset.y;

            fragColor = inColor;
            fragCorner = inCorner;
            gl_Position = uViewProjection * vec4(world, 1.0);
        }
    )";

    inline constexpr auto ParticleFragmentShaderSource = R"(
        #version 460 core

        in vec4 fragColor;
        in vec2 fragCorner;
        out vec4 outColor;

        void main() {
            float fade = 1.0 - smoothstep(0.3, 0.5, length(fragCorner));
            if (fade <= 0.0) discard;
            outColor = vec4(fragColor.rgb, fragColor.a * fade);
        }
    )";
}
