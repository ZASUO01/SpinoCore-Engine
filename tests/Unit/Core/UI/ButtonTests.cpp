// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <gtest/gtest.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include "SpinoCore/Core/Input/InputState.h"
#include "SpinoCore/Core/UI/Elements/Button.h"
#include "SpinoCore/Core/UI/Elements/Text.h"

namespace {
    using SpinoCore::Core::Input::InputState;
    using SpinoCore::Core::UI::Elements::Button;
    using SpinoCore::Core::UI::Elements::TextInput;

    [[nodiscard]] TextInput ButtonAt100x50() {
        TextInput input{};
        input.anchor = {100.0f, 50.0f};
        input.size = {100.0f, 40.0f};
        input.text = "OK";
        return input;
    }

    void MoveMouse(Button& button, const float x, const float y) {
        SDL_Event event{};
        event.type = SDL_EVENT_MOUSE_MOTION;
        event.motion.x = x;
        event.motion.y = y;
        InputState state{};
        state.BeginFrame();
        state.Apply(event);
        button.ProcessInput(state);
    }

    void Click(Button& button, const float x, const float y) {
        SDL_Event motion{};
        motion.type = SDL_EVENT_MOUSE_MOTION;
        motion.motion.x = x;
        motion.motion.y = y;
        SDL_Event press{};
        press.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        press.button.button = SDL_BUTTON_LEFT;
        press.button.down = true;
        press.button.x = x;
        press.button.y = y;
        InputState state{};
        state.BeginFrame();
        state.Apply(motion);
        state.Apply(press);
        button.ProcessInput(state);
    }

    TEST(ButtonTests, ClickingInsideRunsOnClick) {
        Button button{ButtonAt100x50()};
        int clicks = 0;
        button.SetOnClick([&clicks] { ++clicks; });

        Click(button, 140.0f, 60.0f);

        EXPECT_EQ(clicks, 1);
    }

    TEST(ButtonTests, ClickingOutsideDoesNotRunOnClick) {
        Button button{ButtonAt100x50()};
        int clicks = 0;
        button.SetOnClick([&clicks] { ++clicks; });

        Click(button, 160.0f, 60.0f);

        EXPECT_EQ(clicks, 0);
    }

    TEST(ButtonTests, ClickingSelectsTheButton) {
        Button button{ButtonAt100x50()};
        int selections = 0;
        button.SetOnSelect([&selections] { ++selections; });

        Click(button, 100.0f, 50.0f);

        EXPECT_EQ(selections, 1);
    }

    TEST(ButtonTests, ClickingWithoutCallbacksDoesNothing) {
        Button button{ButtonAt100x50()};

        EXPECT_NO_THROW(Click(button, 100.0f, 50.0f));
    }

    TEST(ButtonTests, SelectFromCodeRunsOnSelect) {
        Button button{ButtonAt100x50()};
        int selections = 0;
        button.SetOnSelect([&selections] { ++selections; });

        button.Select();

        EXPECT_EQ(selections, 1);
    }

    TEST(ButtonTests, SelectFromCodeIsNotAClick) {
        Button button{ButtonAt100x50()};
        int clicks = 0;
        button.SetOnClick([&clicks] { ++clicks; });

        button.Select();

        EXPECT_EQ(clicks, 0);
    }

    TEST(ButtonTests, MovingTheMouseOntoTheButtonRunsOnHover) {
        Button button{ButtonAt100x50()};
        int hovers = 0;
        button.SetOnHover([&hovers] { ++hovers; });

        MoveMouse(button, 100.0f, 50.0f);

        EXPECT_EQ(hovers, 1);
    }

    TEST(ButtonTests, KeepingTheMouseOnTheButtonDoesNotRunOnHoverAgain) {
        Button button{ButtonAt100x50()};
        int hovers = 0;
        button.SetOnHover([&hovers] { ++hovers; });
        MoveMouse(button, 100.0f, 50.0f);

        MoveMouse(button, 110.0f, 55.0f);

        EXPECT_EQ(hovers, 1);
    }

    TEST(ButtonTests, LeavingAndComingBackRunsOnHoverAgain) {
        Button button{ButtonAt100x50()};
        int hovers = 0;
        button.SetOnHover([&hovers] { ++hovers; });
        MoveMouse(button, 100.0f, 50.0f);
        MoveMouse(button, 300.0f, 50.0f);

        MoveMouse(button, 100.0f, 50.0f);

        EXPECT_EQ(hovers, 2);
    }

    TEST(ButtonTests, TheButtonIsHoveredWhileTheMouseIsOverIt) {
        Button button{ButtonAt100x50()};

        MoveMouse(button, 100.0f, 50.0f);

        EXPECT_TRUE(button.IsHovered());
    }
}
