// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <gtest/gtest.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include "SpinoCore/Core/Input/InputState.h"

namespace {
    using SpinoCore::Core::Input::InputState;
    using SpinoCore::Core::Input::MouseButton;

    SDL_Event KeyEvent(const SDL_Scancode key, const bool down, const bool repeat = false) {
        SDL_Event event{};
        event.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
        event.key.scancode = key;
        event.key.down = down;
        event.key.repeat = repeat;
        return event;
    }

    SDL_Event MouseButtonEvent(const Uint8 button, const bool down, const float x = 0.0f, const float y = 0.0f) {
        SDL_Event event{};
        event.type = down ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
        event.button.button = button;
        event.button.down = down;
        event.button.x = x;
        event.button.y = y;
        return event;
    }

    SDL_Event MouseMotionEvent(const float x, const float y, const float xrel, const float yrel) {
        SDL_Event event{};
        event.type = SDL_EVENT_MOUSE_MOTION;
        event.motion.x = x;
        event.motion.y = y;
        event.motion.xrel = xrel;
        event.motion.yrel = yrel;
        return event;
    }

    SDL_Event WheelEvent(const float x, const float y, const SDL_MouseWheelDirection direction = SDL_MOUSEWHEEL_NORMAL) {
        SDL_Event event{};
        event.type = SDL_EVENT_MOUSE_WHEEL;
        event.wheel.x = x;
        event.wheel.y = y;
        event.wheel.direction = direction;
        return event;
    }

    SDL_Event TextEvent(const char* text) {
        SDL_Event event{};
        event.type = SDL_EVENT_TEXT_INPUT;
        event.text.text = text;
        return event;
    }

    SDL_Event WindowEvent(const SDL_EventType type) {
        SDL_Event event{};
        event.type = type;
        return event;
    }

    TEST(InputStateTests, NothingIsHeldBeforeAnyEvent) {
        const InputState input{};

        EXPECT_FALSE(input.IsKeyDown(SDL_SCANCODE_SPACE));
    }

    TEST(InputStateTests, AKeyPressedThisFrameIsHeld) {
        InputState input{};
        input.BeginFrame();

        input.Apply(KeyEvent(SDL_SCANCODE_SPACE, true));

        EXPECT_TRUE(input.IsKeyDown(SDL_SCANCODE_SPACE));
    }

    TEST(InputStateTests, AKeyPressedThisFrameIsReportedAsPressed) {
        InputState input{};
        input.BeginFrame();

        input.Apply(KeyEvent(SDL_SCANCODE_SPACE, true));

        EXPECT_TRUE(input.WasKeyPressed(SDL_SCANCODE_SPACE));
    }

    TEST(InputStateTests, PressingOneKeyDoesNotAffectOtherKeys) {
        InputState input{};
        input.BeginFrame();

        input.Apply(KeyEvent(SDL_SCANCODE_SPACE, true));

        EXPECT_FALSE(input.IsKeyDown(SDL_SCANCODE_A));
    }

    TEST(InputStateTests, AKeyHeldSinceAPreviousFrameIsStillHeld) {
        InputState input{};
        input.BeginFrame();
        input.Apply(KeyEvent(SDL_SCANCODE_W, true));

        input.BeginFrame();

        EXPECT_TRUE(input.IsKeyDown(SDL_SCANCODE_W));
    }

    TEST(InputStateTests, AKeyHeldSinceAPreviousFrameIsNotReportedAsPressedAgain) {
        InputState input{};
        input.BeginFrame();
        input.Apply(KeyEvent(SDL_SCANCODE_W, true));

        input.BeginFrame();

        EXPECT_FALSE(input.WasKeyPressed(SDL_SCANCODE_W));
    }

    TEST(InputStateTests, KeyRepeatsFromHoldingAKeyAreNotNewPresses) {
        InputState input{};
        input.BeginFrame();
        input.Apply(KeyEvent(SDL_SCANCODE_W, true));
        input.BeginFrame();

        input.Apply(KeyEvent(SDL_SCANCODE_W, true, true));

        EXPECT_FALSE(input.WasKeyPressed(SDL_SCANCODE_W));
    }

    TEST(InputStateTests, AReleasedKeyIsNoLongerHeld) {
        InputState input{};
        input.BeginFrame();
        input.Apply(KeyEvent(SDL_SCANCODE_W, true));
        input.BeginFrame();

        input.Apply(KeyEvent(SDL_SCANCODE_W, false));

        EXPECT_FALSE(input.IsKeyDown(SDL_SCANCODE_W));
    }

    TEST(InputStateTests, AKeyReleasedThisFrameIsReportedAsReleased) {
        InputState input{};
        input.BeginFrame();
        input.Apply(KeyEvent(SDL_SCANCODE_W, true));
        input.BeginFrame();

        input.Apply(KeyEvent(SDL_SCANCODE_W, false));

        EXPECT_TRUE(input.WasKeyReleased(SDL_SCANCODE_W));
    }

    TEST(InputStateTests, AKeyTappedWithinASingleFrameIsStillReportedAsPressed) {
        InputState input{};
        input.BeginFrame();

        input.Apply(KeyEvent(SDL_SCANCODE_E, true));
        input.Apply(KeyEvent(SDL_SCANCODE_E, false));

        EXPECT_TRUE(input.WasKeyPressed(SDL_SCANCODE_E));
    }

    TEST(InputStateTests, AMouseButtonPressedThisFrameIsHeld) {
        InputState input{};
        input.BeginFrame();

        input.Apply(MouseButtonEvent(SDL_BUTTON_LEFT, true));

        EXPECT_TRUE(input.IsMouseButtonDown(MouseButton::Left));
    }

    TEST(InputStateTests, AMouseButtonPressedThisFrameIsReportedAsPressed) {
        InputState input{};
        input.BeginFrame();

        input.Apply(MouseButtonEvent(SDL_BUTTON_RIGHT, true));

        EXPECT_TRUE(input.WasMouseButtonPressed(MouseButton::Right));
    }

    TEST(InputStateTests, PressingOneMouseButtonDoesNotAffectTheOthers) {
        InputState input{};
        input.BeginFrame();

        input.Apply(MouseButtonEvent(SDL_BUTTON_LEFT, true));

        EXPECT_FALSE(input.IsMouseButtonDown(MouseButton::Right));
    }

    TEST(InputStateTests, AMouseButtonReleasedThisFrameIsReportedAsReleased) {
        InputState input{};
        input.BeginFrame();
        input.Apply(MouseButtonEvent(SDL_BUTTON_MIDDLE, true));
        input.BeginFrame();

        input.Apply(MouseButtonEvent(SDL_BUTTON_MIDDLE, false));

        EXPECT_TRUE(input.WasMouseButtonReleased(MouseButton::Middle));
    }

    TEST(InputStateTests, AMouseButtonClickedWithinASingleFrameIsStillReportedAsPressed) {
        InputState input{};
        input.BeginFrame();

        input.Apply(MouseButtonEvent(SDL_BUTTON_LEFT, true));
        input.Apply(MouseButtonEvent(SDL_BUTTON_LEFT, false));

        EXPECT_TRUE(input.WasMouseButtonPressed(MouseButton::Left));
    }

    TEST(InputStateTests, ClickingMovesTheMouseToWhereTheClickHappened) {
        InputState input{};
        input.BeginFrame();

        input.Apply(MouseButtonEvent(SDL_BUTTON_LEFT, true, 40.0f, 25.0f));

        EXPECT_FLOAT_EQ(input.GetMousePosition().x, 40.0f);
        EXPECT_FLOAT_EQ(input.GetMousePosition().y, 25.0f);
    }

    TEST(InputStateTests, MovingTheMouseUpdatesItsPosition) {
        InputState input{};
        input.BeginFrame();

        input.Apply(MouseMotionEvent(120.0f, 80.0f, 5.0f, -3.0f));

        EXPECT_FLOAT_EQ(input.GetMousePosition().x, 120.0f);
        EXPECT_FLOAT_EQ(input.GetMousePosition().y, 80.0f);
    }

    TEST(InputStateTests, SeveralMouseMovesInOneFrameAddUpToASingleDelta) {
        InputState input{};
        input.BeginFrame();

        input.Apply(MouseMotionEvent(105.0f, 97.0f, 5.0f, -3.0f));
        input.Apply(MouseMotionEvent(107.0f, 101.0f, 2.0f, 4.0f));

        EXPECT_FLOAT_EQ(input.GetMouseDelta().x, 7.0f);
        EXPECT_FLOAT_EQ(input.GetMouseDelta().y, 1.0f);
    }

    TEST(InputStateTests, TheMouseDeltaStartsOverOnTheNextFrame) {
        InputState input{};
        input.BeginFrame();
        input.Apply(MouseMotionEvent(105.0f, 97.0f, 5.0f, -3.0f));

        input.BeginFrame();

        EXPECT_FLOAT_EQ(input.GetMouseDelta().x, 0.0f);
        EXPECT_FLOAT_EQ(input.GetMouseDelta().y, 0.0f);
    }

    TEST(InputStateTests, TheMouseKeepsItsPositionOnFramesWithoutMovement) {
        InputState input{};
        input.BeginFrame();
        input.Apply(MouseMotionEvent(120.0f, 80.0f, 0.0f, 0.0f));

        input.BeginFrame();

        EXPECT_FLOAT_EQ(input.GetMousePosition().x, 120.0f);
        EXPECT_FLOAT_EQ(input.GetMousePosition().y, 80.0f);
    }

    TEST(InputStateTests, SeveralWheelScrollsInOneFrameAddUp) {
        InputState input{};
        input.BeginFrame();

        input.Apply(WheelEvent(0.0f, 1.0f));
        input.Apply(WheelEvent(0.0f, 2.0f));

        EXPECT_FLOAT_EQ(input.GetWheelDelta().y, 3.0f);
    }

    TEST(InputStateTests, AFlippedWheelIsReportedInTheSameDirectionAsANormalOne) {
        InputState input{};
        input.BeginFrame();

        input.Apply(WheelEvent(0.0f, -1.0f, SDL_MOUSEWHEEL_FLIPPED));

        EXPECT_FLOAT_EQ(input.GetWheelDelta().y, 1.0f);
    }

    TEST(InputStateTests, TheMouseIsInsideTheWindowAfterItEnters) {
        InputState input{};
        input.BeginFrame();

        input.Apply(WindowEvent(SDL_EVENT_WINDOW_MOUSE_ENTER));

        EXPECT_TRUE(input.IsMouseInWindow());
    }

    TEST(InputStateTests, TheMouseIsOutsideTheWindowAfterItLeaves) {
        InputState input{};
        input.BeginFrame();
        input.Apply(WindowEvent(SDL_EVENT_WINDOW_MOUSE_ENTER));

        input.Apply(WindowEvent(SDL_EVENT_WINDOW_MOUSE_LEAVE));

        EXPECT_FALSE(input.IsMouseInWindow());
    }

    TEST(InputStateTests, LosingFocusReleasesHeldKeys) {
        InputState input{};
        input.BeginFrame();
        input.Apply(KeyEvent(SDL_SCANCODE_W, true));
        input.BeginFrame();

        input.Apply(WindowEvent(SDL_EVENT_WINDOW_FOCUS_LOST));

        EXPECT_TRUE(input.WasKeyReleased(SDL_SCANCODE_W));
    }

    TEST(InputStateTests, LosingFocusReleasesHeldMouseButtons) {
        InputState input{};
        input.BeginFrame();
        input.Apply(MouseButtonEvent(SDL_BUTTON_LEFT, true));
        input.BeginFrame();

        input.Apply(WindowEvent(SDL_EVENT_WINDOW_FOCUS_LOST));

        EXPECT_FALSE(input.IsMouseButtonDown(MouseButton::Left));
    }

    TEST(InputStateTests, TextTypedThisFrameIsReported) {
        InputState input{};
        input.BeginFrame();

        input.Apply(TextEvent("a"));
        input.Apply(TextEvent("ç"));

        EXPECT_EQ(input.GetTypedText(), "aç");
    }

    TEST(InputStateTests, TypedTextStartsOverOnTheNextFrame) {
        InputState input{};
        input.BeginFrame();
        input.Apply(TextEvent("a"));

        input.BeginFrame();

        EXPECT_TRUE(input.GetTypedText().empty());
    }

    TEST(InputStateTests, AKeyPressedThisFrameIsReportedAsTyped) {
        InputState input{};
        input.BeginFrame();

        input.Apply(KeyEvent(SDL_SCANCODE_BACKSPACE, true));

        EXPECT_TRUE(input.WasKeyTyped(SDL_SCANCODE_BACKSPACE));
    }

    TEST(InputStateTests, KeyRepeatsFromHoldingAKeyAreReportedAsTyped) {
        InputState input{};
        input.BeginFrame();
        input.Apply(KeyEvent(SDL_SCANCODE_BACKSPACE, true));
        input.BeginFrame();

        input.Apply(KeyEvent(SDL_SCANCODE_BACKSPACE, true, true));

        EXPECT_TRUE(input.WasKeyTyped(SDL_SCANCODE_BACKSPACE));
    }

    TEST(InputStateTests, AKeyHeldWithoutRepeatingIsNotTypedAgain) {
        InputState input{};
        input.BeginFrame();
        input.Apply(KeyEvent(SDL_SCANCODE_BACKSPACE, true));

        input.BeginFrame();

        EXPECT_FALSE(input.WasKeyTyped(SDL_SCANCODE_BACKSPACE));
    }
}
