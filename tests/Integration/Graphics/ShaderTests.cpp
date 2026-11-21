// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <array>
#include <cstdint>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include <glad/glad.h>
#include "GLTestContext.h"
#include "Graphics/Geometry/VertexArray.h"
#include "Graphics/Shaders/Shader.h"
#include "Graphics/Shaders/Sources/UIShader.h"
#include "Logs/LogSystem.h"
#include "SpinoCore/Logs/LogLevel.h"
#include "SpinoCore/Logs/LogSink.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/MatrixFactory.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Graphics::Geometry::AttributeType;
    using SpinoCore::Graphics::Geometry::VertexArray;
    using SpinoCore::Graphics::Shaders::Shader;
    using SpinoCore::Logs::Level;
    using SpinoCore::Logs::LogRecord;
    using SpinoCore::Logs::LogSink;
    using SpinoCore::Logs::LogSystem;
    using SpinoCore::Math::Matrix4f;
    using SpinoCore::Math::Vector2f;
    using SpinoCore::Math::Vector3f;
    using SpinoCore::Math::Vector4f;
    using SpinoCore::Tests::GLTestContext;
    using SpinoCore::Tests::Pixel;
    namespace MatrixFactory = SpinoCore::Math::MatrixFactory;
    namespace UI = SpinoCore::Graphics::Shaders::Sources;

    constexpr auto SolidVertexSource = R"(
        #version 460 core
        layout (location = 0) in vec2 inPos;
        layout (location = 0) uniform mat4 uTransform;
        void main() { gl_Position = uTransform * vec4(inPos, 0.0, 1.0); }
    )";

    constexpr auto SolidFragmentSource = R"(
        #version 460 core
        layout (location = 1) uniform vec4 uColor;
        out vec4 outColor;
        void main() { outColor = uColor; }
    )";

    // Every uniform type the shader can set, all used so the compiler keeps them.
    constexpr auto UniformsFragmentSource = R"(
        #version 460 core
        layout (location = 1) uniform vec4 uVec4;
        layout (location = 2) uniform float uFloat;
        layout (location = 3) uniform vec2 uVec2;
        layout (location = 4) uniform vec3 uVec3;
        layout (location = 5) uniform int uInt;
        layout (location = 6) uniform uint uUInt;
        out vec4 outColor;
        void main() { outColor = uVec4 * uFloat + vec4(uVec2, 0.0, 0.0) + vec4(uVec3, float(uInt) + float(uUInt)); }
    )";

    constexpr auto BrokenSource = R"(
        #version 460 core
        void main() { this is not glsl; }
    )";

    // Compiles fine on its own, but a program without main() cannot link.
    constexpr auto FragmentWithoutMainSource = R"(
        #version 460 core
        out vec4 outColor;
        vec4 Helper() { return vec4(1.0); }
    )";

    constexpr Pixel Transparent{0, 0, 0, 0};
    constexpr Pixel White{255, 255, 255, 255};

    struct Vertex2D final {
        float x;
        float y;
    };

    // A quad covering [minX, maxX] x [minY, maxY] in clip space.
    [[nodiscard]] std::unique_ptr<VertexArray> MakeQuad(const float minX, const float minY, const float maxX, const float maxY) {
        const std::array vertices{
            Vertex2D{minX, minY}, Vertex2D{maxX, minY}, Vertex2D{maxX, maxY}, Vertex2D{minX, maxY}
        };
        constexpr std::array<std::uint32_t, 6> indices{0, 1, 2, 2, 3, 0};
        return std::make_unique<VertexArray>(VertexArray::Input{
            .vertexData = std::as_bytes(std::span{vertices}),
            .indices = indices,
            .layout = {{.location = 0, .type = AttributeType::Float2}}
        });
    }

    class CapturingSink final : public LogSink {
    public:
        explicit CapturingSink(std::vector<std::string>& messages) : mMessages(messages) {}

        void Write(const LogRecord& record) override {
            if (record.level == Level::Error) mMessages.emplace_back(record.message);
        }

    private:
        std::vector<std::string>& mMessages;
    };

    using ShaderTests = GLTestContext;

    TEST_F(ShaderTests, ValidSourcesProduceALinkedProgram) {
        const Shader shader{SolidVertexSource, SolidFragmentSource};

        ASSERT_NE(shader.GetID(), 0u);
        GLint linked = GL_FALSE;
        glGetProgramiv(shader.GetID(), GL_LINK_STATUS, &linked);
        EXPECT_EQ(linked, GL_TRUE);
    }

    TEST_F(ShaderTests, TheEngineUIShaderCompilesAndLinks) {
        EXPECT_NO_THROW(Shader(UI::UIVertexShaderSource, UI::UIFragmentShaderSource));
    }

    TEST_F(ShaderTests, AVertexShaderThatDoesNotCompileThrows) {
        EXPECT_THROW(Shader(BrokenSource, SolidFragmentSource), std::runtime_error);
    }

    TEST_F(ShaderTests, AFragmentShaderThatDoesNotCompileThrows) {
        EXPECT_THROW(Shader(SolidVertexSource, BrokenSource), std::runtime_error);
    }

    TEST_F(ShaderTests, AProgramThatDoesNotLinkThrows) {
        EXPECT_THROW(Shader(SolidVertexSource, FragmentWithoutMainSource), std::runtime_error);
    }

    TEST_F(ShaderTests, ACompileErrorIsLoggedWithTheDriverMessageAndNoStrayNullCharacters) {
        std::vector<std::string> errors;
        const LogSystem logSystem{std::make_unique<CapturingSink>(errors)};

        EXPECT_THROW(Shader(BrokenSource, SolidFragmentSource), std::runtime_error);

        ASSERT_EQ(errors.size(), 1u);
        EXPECT_NE(errors[0].find("Compile"), std::string::npos);
        EXPECT_EQ(errors[0].find('\0'), std::string::npos);
    }

    TEST_F(ShaderTests, ALinkErrorIsLogged) {
        std::vector<std::string> errors;
        const LogSystem logSystem{std::make_unique<CapturingSink>(errors)};

        EXPECT_THROW(Shader(SolidVertexSource, FragmentWithoutMainSource), std::runtime_error);

        ASSERT_EQ(errors.size(), 1u);
        EXPECT_NE(errors[0].find("Link"), std::string::npos);
        EXPECT_EQ(errors[0].find('\0'), std::string::npos);
    }

    TEST_F(ShaderTests, DestroyingTheShaderDeletesItsProgram) {
        GLuint program = 0;
        {
            const Shader shader{SolidVertexSource, SolidFragmentSource};
            program = shader.GetID();
            ASSERT_EQ(glIsProgram(program), GL_TRUE);
        }

        EXPECT_EQ(glIsProgram(program), GL_FALSE);
    }

    TEST_F(ShaderTests, SetActiveMakesItTheCurrentProgram) {
        const Shader shader{SolidVertexSource, SolidFragmentSource};

        shader.SetActive();

        GLint current = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &current);
        EXPECT_EQ(static_cast<GLuint>(current), shader.GetID());
        glUseProgram(0);
    }

    TEST_F(ShaderTests, MatricesAreUploadedSoTheShaderSeesTheEngineConvention) {
        const Shader shader{SolidVertexSource, SolidFragmentSource};

        shader.SetUniform(0, MatrixFactory::CreateTranslation(Vector3f{1.0f, 2.0f, 3.0f}));

        std::array<float, 16> uploaded{};
        glGetUniformfv(shader.GetID(), 0, uploaded.data());
        EXPECT_FLOAT_EQ(uploaded[12], 1.0f);
        EXPECT_FLOAT_EQ(uploaded[13], 2.0f);
        EXPECT_FLOAT_EQ(uploaded[14], 3.0f);
        EXPECT_FLOAT_EQ(uploaded[3], 0.0f);
    }

    TEST_F(ShaderTests, ATranslationUniformMovesTheDrawnGeometry) {
        const Shader shader{SolidVertexSource, SolidFragmentSource};
        const auto leftHalf = MakeQuad(-1.0f, -1.0f, 0.0f, 1.0f);

        shader.SetUniform(0, MatrixFactory::CreateTranslation(Vector3f{1.0f, 0.0f, 0.0f}));
        shader.SetUniform(1, Vector4f{1.0f, 1.0f, 1.0f, 1.0f});
        shader.SetActive();
        leftHalf->Draw();
        glUseProgram(0);

        EXPECT_EQ(ReadPixel(2, TargetSize / 2), Transparent);
        EXPECT_EQ(ReadPixel(TargetSize - 2, TargetSize / 2), White);
    }

    TEST_F(ShaderTests, ScalarAndVectorUniformsReachTheProgram) {
        const Shader shader{SolidVertexSource, UniformsFragmentSource};

        shader.SetUniform(1, Vector4f{0.1f, 0.2f, 0.3f, 0.4f});
        shader.SetUniform(2, 0.5f);
        shader.SetUniform(3, Vector2f{0.6f, 0.7f});
        shader.SetUniform(4, Vector3f{0.8f, 0.9f, 1.0f});
        shader.SetUniform(5, -3);
        shader.SetUniform(6, std::uint32_t{7});

        std::array<float, 4> vec4{};
        glGetUniformfv(shader.GetID(), 1, vec4.data());
        EXPECT_EQ(vec4, (std::array{0.1f, 0.2f, 0.3f, 0.4f}));

        float scalar = 0.0f;
        glGetUniformfv(shader.GetID(), 2, &scalar);
        EXPECT_FLOAT_EQ(scalar, 0.5f);

        std::array<float, 2> vec2{};
        glGetUniformfv(shader.GetID(), 3, vec2.data());
        EXPECT_EQ(vec2, (std::array{0.6f, 0.7f}));

        std::array<float, 3> vec3{};
        glGetUniformfv(shader.GetID(), 4, vec3.data());
        EXPECT_EQ(vec3, (std::array{0.8f, 0.9f, 1.0f}));

        GLint integer = 0;
        glGetUniformiv(shader.GetID(), 5, &integer);
        EXPECT_EQ(integer, -3);

        GLuint unsignedInteger = 0;
        glGetUniformuiv(shader.GetID(), 6, &unsignedInteger);
        EXPECT_EQ(unsignedInteger, 7u);
    }

    // The UI shader draws a unit quad and cuts its corners with a rounded box SDF.
    class UIShaderTests : public GLTestContext {
    protected:
        void DrawUIQuad(const float radius) {
            const Shader shader{UI::UIVertexShaderSource, UI::UIFragmentShaderSource};
            const auto unitQuad = MakeQuad(0.0f, 0.0f, 1.0f, 1.0f);
            const auto size = static_cast<float>(TargetSize);

            shader.SetUniform(UI::UIShaderLocation::MVP, MatrixFactory::CreateOrtho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f));
            shader.SetUniform(UI::UIShaderLocation::TextureRect, Vector4f{0.0f, 0.0f, 1.0f, 1.0f});
            shader.SetUniform(UI::UIShaderLocation::TextureMix, 0.0f);
            shader.SetUniform(UI::UIShaderLocation::Color, Vector4f{1.0f, 1.0f, 1.0f, 1.0f});
            shader.SetUniform(UI::UIShaderLocation::Size, Vector2f{size, size});
            shader.SetUniform(UI::UIShaderLocation::Radius, radius);
            shader.SetUniform(UI::UIShaderLocation::Opacity, 1.0f);
            shader.SetActive();
            unitQuad->Draw();
            glUseProgram(0);
        }
    };

    TEST_F(UIShaderTests, WithoutARadiusTheWholeQuadIsFilled) {
        DrawUIQuad(0.0f);

        EXPECT_EQ(ReadPixel(0, 0), White);
        EXPECT_EQ(ReadPixel(TargetSize - 1, TargetSize - 1), White);
        EXPECT_EQ(ReadPixel(TargetSize / 2, TargetSize / 2), White);
    }

    TEST_F(UIShaderTests, ARadiusCutsTheCornersButKeepsTheCenterAndEdges) {
        DrawUIQuad(static_cast<float>(TargetSize) / 2.0f);

        EXPECT_EQ(ReadPixel(0, 0)[3], 0);
        EXPECT_EQ(ReadPixel(TargetSize - 1, TargetSize - 1)[3], 0);
        EXPECT_EQ(ReadPixel(TargetSize / 2, TargetSize / 2), White);
        EXPECT_EQ(ReadPixel(TargetSize / 2, 1)[3], 255);
    }
}
