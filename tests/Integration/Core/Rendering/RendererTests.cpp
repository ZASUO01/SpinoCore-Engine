// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <array>
#include <optional>
#include <gtest/gtest.h>
#include <SDL3/SDL.h>
#include <glad/glad.h>
#include "Core/Events/EventBus.h"
#include "Core/Events/TypeEvents/WindowEvents.h"
#include "Core/Rendering/Renderer.h"
#include "Core/Window/EngineWindow.h"
#include "SDL/Context.h"
#include "SpinoCore/Core/Window/WindowProps.h"
#include "SpinoCore/Core/Window/WindowSize.h"
#include "SpinoCore/Game/GameInfo.h"
#include "SpinoCore/Utils/Size2D.h"

namespace {
    using SpinoCore::Core::Events::EventBus;
    using SpinoCore::Core::Events::WindowResizeEvent;
    using SpinoCore::Core::Rendering::Renderer;
    using SpinoCore::Core::Window::EngineWindow;
    using SpinoCore::Core::Window::WINDOW_SIZE_MIN;
    using SpinoCore::Core::Window::WindowProps;
    using SpinoCore::Game::GameInfo;
    using SpinoCore::SDL::Context;
    using SpinoCore::Utils::Size2D;

    using Viewport = std::array<GLint, 4>;

    [[nodiscard]] Viewport CurrentViewport() {
        Viewport viewport{};
        glGetIntegerv(GL_VIEWPORT, viewport.data());
        return viewport;
    }

    class RendererTests : public testing::Test {
    protected:
        void SetUp() override {
            SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen");
            SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
            context.emplace(GameInfo{});
            window.emplace(WindowProps{.size = WINDOW_SIZE_MIN});
            renderer.emplace(window->GetSDLWindow(), window->GetSize(), bus);
        }

        void DispatchResize(const Size2D pixelSize) {
            WindowResizeEvent event{};
            event.size = pixelSize;
            event.pixelSize = pixelSize;
            bus.Dispatch(event);
        }

        std::optional<Context> context{};
        EventBus bus{};
        std::optional<EngineWindow> window{};
        std::optional<Renderer> renderer{};
    };

    TEST_F(RendererTests, StartsWithAViewportCoveringTheWindow) {
        EXPECT_EQ(CurrentViewport(), (Viewport{0, 0, WINDOW_SIZE_MIN.width, WINDOW_SIZE_MIN.height}));
    }

    TEST_F(RendererTests, ResizingTheWindowResizesTheViewport) {
        DispatchResize({.width = 640, .height = 480});

        EXPECT_EQ(CurrentViewport(), (Viewport{0, 0, 640, 480}));
    }

    TEST_F(RendererTests, IgnoresAResizeToAnEmptyWindow) {
        DispatchResize({.width = 0, .height = 0});

        EXPECT_EQ(CurrentViewport(), (Viewport{0, 0, WINDOW_SIZE_MIN.width, WINDOW_SIZE_MIN.height}));
    }
}
