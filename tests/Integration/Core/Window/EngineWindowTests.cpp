// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <optional>
#include <stdexcept>
#include <utility>
#include <gtest/gtest.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_video.h>
#include "Core/Window/EngineWindow.h"
#include "Core/Window/WindowSizeHelpers.h"
#include "SDL/Context.h"
#include "SpinoCore/Core/Window/WindowProps.h"
#include "SpinoCore/Core/Window/WindowSize.h"
#include "SpinoCore/Game/GameInfo.h"
#include "SpinoCore/Utils/Size2D.h"

namespace {
    using SpinoCore::Core::Window::EngineWindow;
    using SpinoCore::Core::Window::Fits;
    using SpinoCore::Core::Window::IsStandardWindowSize;
    using SpinoCore::Core::Window::Size2D;
    using SpinoCore::Core::Window::WINDOW_SIZE_MAX;
    using SpinoCore::Core::Window::WINDOW_SIZE_MIN;
    using SpinoCore::Core::Window::WindowMode;
    using SpinoCore::Core::Window::WindowProps;
    using SpinoCore::Game::GameInfo;
    using SpinoCore::SDL::Context;

    [[nodiscard]] Size2D UsableDisplayArea() {
        SDL_Rect bounds{};
        SDL_GetDisplayUsableBounds(SDL_GetPrimaryDisplay(), &bounds);
        return {.width = bounds.w, .height = bounds.h};
    }

    class EngineWindowTests : public testing::Test {
    protected:
        void SetUp() override {
            SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen");
            SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
            context.emplace(GameInfo{});
        }

        std::optional<Context> context{};
    };

    TEST_F(EngineWindowTests, RejectsAnEmptyTitle) {
        EXPECT_THROW(EngineWindow(WindowProps{.title = ""}), std::invalid_argument);
    }

    TEST_F(EngineWindowTests, OpensWithTheRequestedSizeWhenItFits) {
        const EngineWindow window{WindowProps{.size = WINDOW_SIZE_MIN}};

        EXPECT_EQ(window.GetSize(), WINDOW_SIZE_MIN);
    }

    TEST_F(EngineWindowTests, OpensWithAStandardSizeThatFitsTheDisplayWhenTheRequestedOneDoesNot) {
        const EngineWindow window{WindowProps{.size = WINDOW_SIZE_MAX}};

        const Size2D size = window.GetSize();

        EXPECT_TRUE(IsStandardWindowSize(size));
        EXPECT_TRUE(Fits(size, UsableDisplayArea()));
    }

    TEST_F(EngineWindowTests, OpensInWindowedMode) {
        const EngineWindow window{WindowProps{}};

        EXPECT_EQ(window.GetMode(), WindowMode::Windowed);
    }

    TEST_F(EngineWindowTests, RefusesANonStandardSize) {
        EngineWindow window{WindowProps{.size = WINDOW_SIZE_MIN}};

        const bool applied = window.SetSize({.width = 1000, .height = 700});

        EXPECT_FALSE(applied);
        EXPECT_EQ(window.GetSize(), WINDOW_SIZE_MIN);
    }

    TEST_F(EngineWindowTests, ShowsTheSystemCursorByDefault) {
        const EngineWindow window{WindowProps{}};

        EXPECT_TRUE(window.IsSystemCursorVisible());
    }

    TEST_F(EngineWindowTests, HidesTheSystemCursorWhenAsked) {
        EngineWindow window{WindowProps{}};

        window.SetSystemCursorVisible(false);

        EXPECT_FALSE(window.IsSystemCursorVisible());
    }

    TEST_F(EngineWindowTests, ShowsTheSystemCursorAgainWhenAsked) {
        EngineWindow window{WindowProps{}};
        window.SetSystemCursorVisible(false);

        window.SetSystemCursorVisible(true);

        EXPECT_TRUE(window.IsSystemCursorVisible());
    }

    TEST_F(EngineWindowTests, ClosingTheWindowShowsTheSystemCursorAgain) {
        std::optional<EngineWindow> window{std::in_place, WindowProps{}};
        window->SetSystemCursorVisible(false);

        window.reset();

        EXPECT_TRUE(SDL_CursorVisible());
    }
}
