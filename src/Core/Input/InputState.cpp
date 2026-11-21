// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "InputState.h"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>

namespace SpinoCore::Core::Input {
    namespace {
        constexpr std::size_t ToIndex(const MouseButton button) noexcept {
            return static_cast<std::size_t>(button);
        }
    }

    void InputState::BeginFrame() noexcept {
        mKeysPressed.reset();
        mKeysReleased.reset();
        mMouseButtonsPressed.reset();
        mMouseButtonsReleased.reset();
        mMouseDelta = Math::Vector2f::Zero();
        mWheelDelta = Math::Vector2f::Zero();
    }

    void InputState::Apply(const SDL_Event& event) noexcept {
        switch (event.type) {
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                SetKey(event.key.scancode, event.key.down, event.key.repeat);
                break;
            case SDL_EVENT_MOUSE_MOTION:
                mMousePosition = {event.motion.x, event.motion.y};
                mMouseDelta += {event.motion.xrel, event.motion.yrel};
                mIsMouseInWindow = true;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                mMousePosition = {event.button.x, event.button.y};
                SetMouseButton(event.button.button, event.button.down);
                break;
            case SDL_EVENT_MOUSE_WHEEL: {
                const float direction = event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1.0f : 1.0f;
                mWheelDelta += Math::Vector2f{event.wheel.x, event.wheel.y} * direction;
                break;
            }
            case SDL_EVENT_WINDOW_MOUSE_ENTER:
                mIsMouseInWindow = true;
                break;
            case SDL_EVENT_WINDOW_MOUSE_LEAVE:
                mIsMouseInWindow = false;
                break;
            case SDL_EVENT_WINDOW_FOCUS_LOST:
                ReleaseAll();
                break;
            default:
                break;
        }
    }

    void InputState::ReleaseAll() noexcept {
        mKeysReleased |= mKeysDown;
        mKeysDown.reset();
        mMouseButtonsReleased |= mMouseButtonsDown;
        mMouseButtonsDown.reset();
    }

    bool InputState::IsKeyDown(const SDL_Scancode key) const noexcept {
        return key < KEY_COUNT && mKeysDown.test(key);
    }

    bool InputState::WasKeyPressed(const SDL_Scancode key) const noexcept {
        return key < KEY_COUNT && mKeysPressed.test(key);
    }

    bool InputState::WasKeyReleased(const SDL_Scancode key) const noexcept {
        return key < KEY_COUNT && mKeysReleased.test(key);
    }

    bool InputState::IsMouseButtonDown(const MouseButton button) const noexcept {
        return ToIndex(button) < MOUSE_BUTTON_COUNT && mMouseButtonsDown.test(ToIndex(button));
    }

    bool InputState::WasMouseButtonPressed(const MouseButton button) const noexcept {
        return ToIndex(button) < MOUSE_BUTTON_COUNT && mMouseButtonsPressed.test(ToIndex(button));
    }

    bool InputState::WasMouseButtonReleased(const MouseButton button) const noexcept {
        return ToIndex(button) < MOUSE_BUTTON_COUNT && mMouseButtonsReleased.test(ToIndex(button));
    }

    void InputState::SetKey(const SDL_Scancode key, const bool down, const bool repeat) noexcept {
        if (key >= KEY_COUNT || repeat) return;

        if (down && !mKeysDown.test(key)) mKeysPressed.set(key);
        if (!down && mKeysDown.test(key)) mKeysReleased.set(key);
        mKeysDown.set(key, down);
    }

    void InputState::SetMouseButton(const std::uint8_t sdlButton, const bool down) noexcept {
        if (sdlButton < SDL_BUTTON_LEFT || sdlButton > SDL_BUTTON_X2) return;
        const std::size_t index = sdlButton - SDL_BUTTON_LEFT;

        if (down && !mMouseButtonsDown.test(index)) mMouseButtonsPressed.set(index);
        if (!down && mMouseButtonsDown.test(index)) mMouseButtonsReleased.set(index);
        mMouseButtonsDown.set(index, down);
    }
}
