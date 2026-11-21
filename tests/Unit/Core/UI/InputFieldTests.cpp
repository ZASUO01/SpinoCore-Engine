// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <string>
#include <gtest/gtest.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include "Core/Input/InputState.h"
#include "SpinoCore/Core/UI/Elements/InputField.h"

namespace {
    using SpinoCore::Core::Input::InputState;
    using SpinoCore::Core::UI::Elements::InputField;
    using SpinoCore::Core::UI::Elements::InputFieldInput;

    // A 200x40 field centered at (100, 50): it covers x in [0, 200] and y in [30, 70].
    [[nodiscard]] InputFieldInput FieldAt100x50(const std::string& text = {}) {
        InputFieldInput input{};
        input.anchor = {100.0f, 50.0f};
        input.size = {200.0f, 40.0f};
        input.text = text;
        return input;
    }

    void Click(InputField& field, const float x, const float y) {
        SDL_Event event{};
        event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        event.button.button = SDL_BUTTON_LEFT;
        event.button.down = true;
        event.button.x = x;
        event.button.y = y;
        InputState state{};
        state.BeginFrame();
        state.Apply(event);
        field.ProcessInput(state);
    }

    void Type(InputField& field, const char* text) {
        SDL_Event event{};
        event.type = SDL_EVENT_TEXT_INPUT;
        event.text.text = text;
        InputState state{};
        state.BeginFrame();
        state.Apply(event);
        field.ProcessInput(state);
    }

    void PressBackspace(InputField& field) {
        SDL_Event event{};
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.scancode = SDL_SCANCODE_BACKSPACE;
        event.key.down = true;
        InputState state{};
        state.BeginFrame();
        state.Apply(event);
        field.ProcessInput(state);
    }

    // Focus

    TEST(InputFieldTests, StartsWithoutFocus) {
        const InputField field{FieldAt100x50()};

        EXPECT_FALSE(field.IsFocused());
    }

    TEST(InputFieldTests, ClickingInsideTheFieldFocusesIt) {
        InputField field{FieldAt100x50()};

        Click(field, 190.0f, 60.0f);

        EXPECT_TRUE(field.IsFocused());
    }

    TEST(InputFieldTests, ClickingOutsideTheFieldReleasesTheFocus) {
        InputField field{FieldAt100x50()};
        Click(field, 100.0f, 50.0f);

        Click(field, 100.0f, 80.0f);

        EXPECT_FALSE(field.IsFocused());
    }

    TEST(InputFieldTests, ClicksFollowTheFieldScale) {
        InputField field{FieldAt100x50()};
        field.SetScale(2.0f);

        Click(field, 100.0f, 85.0f);

        EXPECT_TRUE(field.IsFocused());
    }

    // Typing

    TEST(InputFieldTests, TypingWhileFocusedAppendsToTheText) {
        InputField field{FieldAt100x50("ol")};
        Click(field, 100.0f, 50.0f);

        Type(field, "á!");

        EXPECT_EQ(field.GetText(), "olá!");
    }

    TEST(InputFieldTests, TypingWithoutFocusIsIgnored) {
        InputField field{FieldAt100x50("ol")};

        Type(field, "á");

        EXPECT_EQ(field.GetText(), "ol");
    }

    TEST(InputFieldTests, BackspaceRemovesTheLastCharacter) {
        InputField field{FieldAt100x50("abc")};
        Click(field, 100.0f, 50.0f);

        PressBackspace(field);

        EXPECT_EQ(field.GetText(), "ab");
    }

    TEST(InputFieldTests, BackspaceRemovesAnAccentedCharacterWhole) {
        InputField field{FieldAt100x50("pé")};
        Click(field, 100.0f, 50.0f);

        PressBackspace(field);

        EXPECT_EQ(field.GetText(), "p");
    }

    TEST(InputFieldTests, BackspaceOnAnEmptyFieldDoesNothing) {
        InputField field{FieldAt100x50()};
        Click(field, 100.0f, 50.0f);

        PressBackspace(field);

        EXPECT_EQ(field.GetText(), "");
    }

    // Length limit

    TEST(InputFieldTests, TypingStopsAtOneHundredCharacters) {
        InputField field{FieldAt100x50(std::string(99, 'a'))};
        Click(field, 100.0f, 50.0f);

        Type(field, "bc");

        EXPECT_EQ(field.GetText(), std::string(99, 'a') + "b");
    }

    TEST(InputFieldTests, TheLimitCountsCharactersNotBytes) {
        std::string accented;
        for (int i = 0; i < 99; ++i) accented += "é";
        InputField field{FieldAt100x50(accented)};
        Click(field, 100.0f, 50.0f);

        Type(field, "ãõ");

        EXPECT_EQ(field.GetText(), accented + "ã");
    }

    TEST(InputFieldTests, TextSetFromCodeIsCutAtOneHundredCharacters) {
        InputField field{FieldAt100x50()};

        field.SetText(std::string(150, 'x'));

        EXPECT_EQ(field.GetText(), std::string(InputField::MAX_LENGTH, 'x'));
    }
}
