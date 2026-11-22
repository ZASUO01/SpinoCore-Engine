// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Graphics {
    enum class UniformType : std::uint8_t {
        Float,
        Int,
        Vec2,
        Vec3,
        Vec4,
        Mat4
    };

    using UniformValue = std::variant<float, int, Math::Vector2f, Math::Vector3f, Math::Vector4f, Math::Matrix4f>;

    struct PostProcessUniform {
        UniformType type{UniformType::Float};
        UniformValue value{0.0f};
    };

    struct PostProcess {
        std::string fragmentShader;
        std::vector<PostProcessUniform> uniforms;
    };

    namespace PostProcessLocation {
        inline constexpr std::int32_t MaxUserUniforms = 100;
        inline constexpr std::int32_t SceneTexture = 100;
        inline constexpr std::int32_t Resolution = 101;
        inline constexpr std::int32_t Time = 102;
    }

    namespace DefaultPostProcess {
        inline constexpr auto Grayscale = "Grayscale";
        inline constexpr auto Vignette = "Vignette";
        inline constexpr auto Exposure = "Exposure";
    }

    [[nodiscard]] constexpr bool Matches(const PostProcessUniform& uniform) noexcept {
        return static_cast<std::size_t>(uniform.type) == uniform.value.index();
    }
}
