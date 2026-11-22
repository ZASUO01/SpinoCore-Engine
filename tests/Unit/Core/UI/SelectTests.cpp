// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include "SpinoCore/Core/Input/InputState.h"
#include "SpinoCore/Core/UI/Elements/Select.h"
#include "SpinoCore/Core/UI/Elements/Text.h"

namespace {
    using SpinoCore::Core::Input::InputState;
    using SpinoCore::Core::UI::Elements::Select;
    using SpinoCore::Core::UI::Elements::TextInput;

    [[nodiscard]] TextInput SelectAt200x50() {
        TextInput input{};
        input.anchor = {200.0f, 50.0f};
        input.size = {120.0f, 40.0f};
        return input;
    }

    const std::vector<std::string> Difficulties{"Fácil", "Normal", "Difícil"};

    void Click(Select& select, const float x, const float y) {
        SDL_Event press{};
        press.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        press.button.button = SDL_BUTTON_LEFT;
        press.button.down = true;
        press.button.x = x;
        press.button.y = y;
        SDL_Event enter{};
        enter.type = SDL_EVENT_WINDOW_MOUSE_ENTER;
        InputState state{};
        state.BeginFrame();
        state.Apply(enter);
        state.Apply(press);
        select.ProcessInput(state);
    }

    TEST(SelectTests, StartsOnTheFirstOption) {
        const Select select{SelectAt200x50(), Difficulties};

        EXPECT_EQ(select.GetSelectedOption(), "Fácil");
    }

    TEST(SelectTests, ShowsTheSelectedOption) {
        Select select{SelectAt200x50(), Difficulties};

        select.SetSelectedIndex(2);

        EXPECT_EQ(select.GetText(), "Difícil");
    }

    TEST(SelectTests, ClickingTheRightArrowShowsTheNextOption) {
        Select select{SelectAt200x50(), Difficulties};

        Click(select, 280.0f, 50.0f);

        EXPECT_EQ(select.GetText(), "Normal");
    }

    TEST(SelectTests, ClickingTheLeftArrowShowsThePreviousOption) {
        Select select{SelectAt200x50(), Difficulties};
        select.SetSelectedIndex(2);

        Click(select, 120.0f, 50.0f);

        EXPECT_EQ(select.GetText(), "Normal");
    }

    TEST(SelectTests, GoingBackFromTheFirstOptionWrapsToTheLast) {
        Select select{SelectAt200x50(), Difficulties};

        Click(select, 120.0f, 50.0f);

        EXPECT_EQ(select.GetSelectedOption(), "Difícil");
    }

    TEST(SelectTests, GoingForwardFromTheLastOptionWrapsToTheFirst) {
        Select select{SelectAt200x50(), Difficulties};
        select.SetSelectedIndex(2);

        Click(select, 280.0f, 50.0f);

        EXPECT_EQ(select.GetSelectedOption(), "Fácil");
    }

    TEST(SelectTests, ClickingTheOptionItselfChangesNothing) {
        Select select{SelectAt200x50(), Difficulties};

        Click(select, 200.0f, 50.0f);

        EXPECT_EQ(select.GetSelectedIndex(), 0u);
    }

    TEST(SelectTests, AnIndexPastTheEndIsIgnored) {
        Select select{SelectAt200x50(), Difficulties};

        select.SetSelectedIndex(3);

        EXPECT_EQ(select.GetSelectedIndex(), 0u);
    }

    TEST(SelectTests, ASelectWithoutOptionsCannotBeCreated) {
        EXPECT_THROW((Select{SelectAt200x50(), {}}), std::invalid_argument);
    }
}
