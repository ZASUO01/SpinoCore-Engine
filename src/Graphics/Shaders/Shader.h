// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <string>
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Graphics::Shaders {
    class Shader final  {
    public:
        Shader(const std::string& vertexSource, const std::string& fragmentSource);
        ~Shader();

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&&) = delete;
        Shader& operator=(Shader&&) = delete;

        [[nodiscard]] std::uint32_t GetID() const noexcept { return mProgram; }
        void SetActive() const noexcept;

        void SetUniform(std::int32_t location, const Math::Vector2f& vector) const noexcept;
        void SetUniform(std::int32_t location, const Math::Vector3f& vector) const noexcept;
        void SetUniform(std::int32_t location, const Math::Vector4f& vector) const noexcept;
        void SetUniform(std::int32_t location, const Math::Matrix4f& matrix) const noexcept;
        void SetUniform(std::int32_t location, float value) const noexcept;
        void SetUniform(std::int32_t location, int value) const noexcept;
        void SetUniform(std::int32_t location, std::uint32_t value) const noexcept;

    private:
        std::uint32_t mProgram{0};
    };
}