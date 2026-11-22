// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>

namespace SpinoCore::Graphics::Shaders::Sources {
    namespace ModelShaderLocation {
        inline constexpr std::int32_t Model = 0;
        inline constexpr std::int32_t ViewProjection = 1;
        inline constexpr std::int32_t BaseColor = 2;
        inline constexpr std::int32_t LightDirection = 3;
        inline constexpr std::int32_t Skinned = 4;
        inline constexpr std::int32_t Bones = 5;
        inline constexpr std::int32_t LightViewProjection = 69;
        inline constexpr std::int32_t GlobalLight = 70;
        inline constexpr std::int32_t LightColor = 71;
        inline constexpr std::int32_t AmbientColor = 72;
    }

    namespace ModelShaderTextureUnit {
        inline constexpr std::uint32_t Albedo = 0;
        inline constexpr std::uint32_t ShadowMap = 1;
    }

    inline constexpr auto ModelSkinningPreludeSource = R"(
        #version 460 core

        layout (location = 0) in vec3 inPosition;
        layout (location = 1) in vec3 inNormal;
        layout (location = 2) in vec2 inTextureCoord;
        layout (location = 3) in uvec4 inJoints;
        layout (location = 4) in vec4 inWeights;

        layout (location = 0) uniform mat4 uModel;
        layout (location = 1) uniform mat4 uViewProjection;
        layout (location = 4) uniform int uSkinned;
        layout (location = 5) uniform mat4 uBones[64];

        mat4 WorldMatrix() {
            if (uSkinned == 0) return uModel;
            return uModel * (inWeights.x * uBones[inJoints.x]
                           + inWeights.y * uBones[inJoints.y]
                           + inWeights.z * uBones[inJoints.z]
                           + inWeights.w * uBones[inJoints.w]);
        }
    )";

    inline constexpr auto ModelVertexShaderBody = R"(
        layout (location = 69) uniform mat4 uLightViewProjection;

        out vec3 fragNormal;
        out vec2 fragTextureCoord;
        out vec4 fragLightSpace;

        void main() {
            mat4 world = WorldMatrix();
            vec4 worldPosition = world * vec4(inPosition, 1.0);
            fragNormal = mat3(world) * inNormal;
            fragTextureCoord = inTextureCoord;
            fragLightSpace = uLightViewProjection * worldPosition;
            gl_Position = uViewProjection * worldPosition;
        }
    )";

    inline constexpr auto ModelFragmentShaderSource = R"(
        #version 460 core

        in vec3 fragNormal;
        in vec2 fragTextureCoord;
        in vec4 fragLightSpace;

        layout (location = 2) uniform vec4 uBaseColor;
        layout (location = 3) uniform vec3 uLightDirection;
        layout (location = 70) uniform int uGlobalLight;
        layout (location = 71) uniform vec3 uLightColor;
        layout (location = 72) uniform vec3 uAmbientColor;
        layout (binding = 0) uniform sampler2D uTexture;
        layout (binding = 1) uniform sampler2D uShadowMap;

        out vec4 outColor;

        float Visibility(float facing) {
            vec3 projected = fragLightSpace.xyz / fragLightSpace.w * 0.5 + 0.5;
            if (projected.z > 1.0 || projected.x < 0.0 || projected.x > 1.0 || projected.y < 0.0 || projected.y > 1.0) {
                return 1.0;
            }

            float bias = max(0.004 * (1.0 - facing), 0.0008);
            vec2 texelSize = 1.0 / vec2(textureSize(uShadowMap, 0));
            float lit = 0.0;
            for (int x = -1; x <= 1; ++x) {
                for (int y = -1; y <= 1; ++y) {
                    float closest = texture(uShadowMap, projected.xy + vec2(x, y) * texelSize).r;
                    lit += projected.z - bias > closest ? 0.0 : 1.0;
                }
            }
            return lit / 9.0;
        }

        void main() {
            vec4 texel = texture(uTexture, fragTextureCoord);
            vec4 albedo = uBaseColor * vec4(pow(texel.rgb, vec3(2.2)), texel.a);

            bool hasNormal = dot(fragNormal, fragNormal) > 0.0;
            float facing = hasNormal ? max(dot(normalize(fragNormal), -normalize(uLightDirection)), 0.0) : 1.0;

            vec3 light = uGlobalLight != 0
                ? uAmbientColor + uLightColor * facing * Visibility(facing)
                : vec3(0.3 + 0.7 * facing);
            outColor = vec4(pow(albedo.rgb * light, vec3(1.0 / 2.2)), albedo.a);
        }
    )";

    inline constexpr auto ShadowVertexShaderBody = R"(
        void main() {
            gl_Position = uViewProjection * WorldMatrix() * vec4(inPosition, 1.0);
        }
    )";

    inline constexpr auto ShadowFragmentShaderSource = R"(
        #version 460 core

        void main() {}
    )";
}
