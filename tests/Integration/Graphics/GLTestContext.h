// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <gtest/gtest.h>
#include <SDL3/SDL.h>
#include <glad/glad.h>
#include "Core/Events/EventBus.h"
#include "Core/Rendering/Renderer.h"
#include "Core/Window/EngineWindow.h"
#include "SDL/Context.h"
#include "SpinoCore/Core/Window/WindowProps.h"
#include "SpinoCore/Core/Window/WindowSize.h"
#include "SpinoCore/Game/GameInfo.h"

namespace SpinoCore::Tests {
    using Pixel = std::array<std::uint8_t, 4>;

    class GLTestContext : public testing::Test {
    protected:
        static constexpr GLsizei TargetSize = 16;

        void SetUp() override {
            SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen");
            SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
            context.emplace(Game::GameInfo{});
            window.emplace(Core::Window::WindowProps{.size = Core::Window::WINDOW_SIZE_MIN});
            renderer.emplace(window->GetSDLWindow(), window->GetSize(), bus);

            glCreateRenderbuffers(1, &colorBuffer);
            glNamedRenderbufferStorage(colorBuffer, GL_RGBA8, TargetSize, TargetSize);
            glCreateFramebuffers(1, &framebuffer);
            glNamedFramebufferRenderbuffer(framebuffer, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer);
            ASSERT_EQ(glCheckNamedFramebufferStatus(framebuffer, GL_FRAMEBUFFER), static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE));

            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glViewport(0, 0, TargetSize, TargetSize);
            glDisable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        void TearDown() override {
            if (!renderer) return;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteFramebuffers(1, &framebuffer);
            glDeleteRenderbuffers(1, &colorBuffer);
        }

        [[nodiscard]] static Pixel ReadPixel(const GLint x, const GLint y) {
            Pixel pixel{};
            glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel.data());
            return pixel;
        }

        std::optional<SDL::Context> context{};
        Core::Events::EventBus bus{};
        std::optional<Core::Window::EngineWindow> window{};
        std::optional<Core::Rendering::Renderer> renderer{};
        GLuint colorBuffer{0};
        GLuint framebuffer{0};
    };
}
