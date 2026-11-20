// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "ShaderHelpers.h"
#include "glad/glad.h"
#include "SpinoCore/Logs/Logger.h"

namespace SpinoCore::Graphics::Shaders {
    namespace {
        [[nodiscard]] std::string GetShaderInfoLog(const GLuint shaderID) {
            GLint logLength = 0;
            glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength <= 1) return {};

            std::string log;
            log.resize(static_cast<size_t>(logLength));
            glGetShaderInfoLog(shaderID, logLength, nullptr, log.data());
            return log;
        }

        [[nodiscard]] std::string GetProgramInfoLog(const GLuint programID) {
            GLint logLength = 0;
            glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength <= 1) return {};

            std::string log;
            log.resize(static_cast<size_t>(logLength));
            glGetProgramInfoLog(programID, logLength, nullptr, log.data());
            return log;
        }

        [[nodiscard]] GLuint CompileShader(const std::string_view shaderSource, const GLenum shaderType) {
            const char* contentsChar = shaderSource.data();

            const GLuint shaderID = glCreateShader(shaderType);
            const auto length = static_cast<GLint>(shaderSource.length());

            glShaderSource(shaderID, 1, &contentsChar, &length);
            glCompileShader(shaderID);


            GLint status = 0;
            glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
            if (status != GL_TRUE) {
                Logs::Logger::Error("[SHADER] GLSL Compile Failed:\n{}", GetShaderInfoLog(shaderID));
                glDeleteShader(shaderID);
                return 0;
            }

            return shaderID;
        }
    }

    GLuint BuildFromStrings(const std::string& vertexShaderSource, const std::string& fragmentShaderSource) {
        const GLuint vertexShaderID = CompileShader(vertexShaderSource, GL_VERTEX_SHADER);
        if (vertexShaderID == 0) return 0;

        const GLuint fragmentShaderID = CompileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
        if (fragmentShaderID == 0) {
            glDeleteShader(vertexShaderID);
            return 0;
        }

        const GLuint program = glCreateProgram();

        glAttachShader(program, vertexShaderID);
        glAttachShader(program, fragmentShaderID);
        glLinkProgram(program);

        glDetachShader(program, vertexShaderID);
        glDetachShader(program, fragmentShaderID);
        glDeleteShader(vertexShaderID);
        glDeleteShader(fragmentShaderID);

        GLint status = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (status != GL_TRUE) {
            Logs::Logger::Error("[SHADER] GLSL Link Status:\n{}", GetProgramInfoLog(program));
            glDeleteProgram(program);
            return 0;
        }

        return program;
    }
}
