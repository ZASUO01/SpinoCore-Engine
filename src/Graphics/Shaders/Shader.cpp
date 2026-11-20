// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Shader.h"
#include <stdexcept>
#include <glad/glad.h>
#include "ShaderHelpers.h"

namespace SpinoCore::Graphics::Shaders {
    Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource) {
        const auto shaderProgram = BuildFromStrings(vertexSource, fragmentSource);
        if (shaderProgram == 0) throw std::runtime_error("[SHADER] Invalid shader program");

        mProgram = shaderProgram;
    }

    Shader::~Shader() {
        if (mProgram != 0) {
            glDeleteProgram(mProgram);
            mProgram = 0;
        }
    }

    void Shader::SetActive() const noexcept {
        glUseProgram(mProgram);
    }

    void Shader::SetUniform(const std::int32_t location, const Math::Vector2f& vector) const noexcept {
        glProgramUniform2fv(mProgram, location, 1, vector.Data());
    }

    void Shader::SetUniform (const std::int32_t location, const Math::Vector3f& vector) const noexcept {
        glProgramUniform3fv(mProgram, location, 1, vector.Data());
    }

    void Shader::SetUniform (const std::int32_t location, const Math::Vector4f& vector) const noexcept {
        glProgramUniform4fv(mProgram, location, 1, vector.Data());
    }

    void Shader::SetUniform(const std::int32_t location, const Math::Matrix4f& matrix) const noexcept {
        glProgramUniformMatrix4fv(mProgram, location, 1, GL_FALSE, matrix.Data());
    }

    void Shader::SetUniform(const std::int32_t location, const float value) const noexcept {
        glProgramUniform1f(mProgram, location, value);
    }

    void Shader::SetUniform(const std::int32_t location, const int value) const noexcept{
        glProgramUniform1i(mProgram, location, value);
    }

    void Shader::SetUniform(const std::int32_t location, const std::uint32_t value) const noexcept {
        glProgramUniform1ui(mProgram, location, value);
    }
}
