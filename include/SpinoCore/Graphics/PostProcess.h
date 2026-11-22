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

/// @brief Full-screen post-processing effects.
namespace SpinoCore::Graphics {
    /// @brief Type of a post-process uniform; its order matches UniformValue.
    enum class UniformType : std::uint8_t {
        Float,
        Int,
        Vec2,
        Vec3,
        Vec4,
        Mat4
    };

    /// @brief Value of a post-process uniform.
    using UniformValue = std::variant<float, int, Math::Vector2f, Math::Vector3f, Math::Vector4f, Math::Matrix4f>;

    /// @brief A uniform of a post-process; its index in the list is its GLSL location.
    struct PostProcessUniform {
        /// Declared type.
        UniformType type{UniformType::Float};
        /// Current value; it must hold the declared type.
        UniformValue value{0.0f};
    };

    /// @brief A full-screen effect applied to the rendered scene.
    struct PostProcess {
        /// GLSL code with a main function. The engine adds the version line, the input
        /// fragTextureCoord, the output outColor and the uniforms uScene, uResolution and uTime.
        std::string fragmentShader;
        /// User uniforms at locations 0, 1, 2 and so on.
        std::vector<PostProcessUniform> uniforms;
    };

    /// @brief GLSL uniform locations used by post-processes.
    namespace PostProcessLocation {
        /// Number of locations available for user uniforms.
        inline constexpr std::int32_t MaxUserUniforms = 100;
        /// Location of uScene, the rendered scene.
        inline constexpr std::int32_t SceneTexture = 100;
        /// Location of uResolution, the viewport size in pixels.
        inline constexpr std::int32_t Resolution = 101;
        /// Location of uTime, the seconds since the engine started.
        inline constexpr std::int32_t Time = 102;
    }

    /// @brief Names of the post-processes that are always available.
    namespace DefaultPostProcess {
        /// Desaturates the scene; uniform 0 is the strength from 0 to 1.
        inline constexpr auto Grayscale = "Grayscale";
        /// Darkens the edges; uniform 0 is the radius and uniform 1 the softness.
        inline constexpr auto Vignette = "Vignette";
        /// Brightness and gamma; uniform 0 is the exposure and uniform 1 the gamma.
        inline constexpr auto Exposure = "Exposure";
    }

    /// @param uniform Uniform to check.
    /// @return Whether the value holds the declared type.
    [[nodiscard]] constexpr bool Matches(const PostProcessUniform& uniform) noexcept {
        return static_cast<std::size_t>(uniform.type) == uniform.value.index();
    }
}
