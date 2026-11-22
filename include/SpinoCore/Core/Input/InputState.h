// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <string>
#include <SDL3/SDL_scancode.h>
#include "SpinoCore/Math/Vector.h"

union SDL_Event;

namespace SpinoCore::Core::Input {
    enum class MouseButton : std::uint8_t { Left, Middle, Right, X1, X2 };

    class InputState final {
    public:
        void BeginFrame() noexcept;
        void Apply(const SDL_Event& event) noexcept;
        void ReleaseAll() noexcept;
        void Accumulate(const InputState& frame);

        [[nodiscard]] bool IsKeyDown(SDL_Scancode key) const noexcept;
        [[nodiscard]] bool WasKeyPressed(SDL_Scancode key) const noexcept;
        [[nodiscard]] bool WasKeyReleased(SDL_Scancode key) const noexcept;
        [[nodiscard]] bool WasKeyTyped(SDL_Scancode key) const noexcept;

        [[nodiscard]] const std::string& GetTypedText() const noexcept { return mTypedText; }

        [[nodiscard]] bool IsMouseButtonDown(MouseButton button) const noexcept;
        [[nodiscard]] bool WasMouseButtonPressed(MouseButton button) const noexcept;
        [[nodiscard]] bool WasMouseButtonReleased(MouseButton button) const noexcept;

        [[nodiscard]] const Math::Vector2f& GetMousePosition() const noexcept { return mMousePosition; }
        [[nodiscard]] const Math::Vector2f& GetMouseDelta() const noexcept { return mMouseDelta; }
        [[nodiscard]] const Math::Vector2f& GetWheelDelta() const noexcept { return mWheelDelta; }
        [[nodiscard]] bool IsMouseInWindow() const noexcept { return mIsMouseInWindow; }

    private:
        static constexpr std::size_t KEY_COUNT = SDL_SCANCODE_COUNT;
        static constexpr std::size_t MOUSE_BUTTON_COUNT = 5;

        void SetKey(SDL_Scancode key, bool down, bool repeat) noexcept;
        void SetMouseButton(std::uint8_t sdlButton, bool down) noexcept;

        std::bitset<KEY_COUNT> mKeysDown{};
        std::bitset<KEY_COUNT> mKeysPressed{};
        std::bitset<KEY_COUNT> mKeysReleased{};
        std::bitset<KEY_COUNT> mKeysTyped{};
        std::string mTypedText{};

        std::bitset<MOUSE_BUTTON_COUNT> mMouseButtonsDown{};
        std::bitset<MOUSE_BUTTON_COUNT> mMouseButtonsPressed{};
        std::bitset<MOUSE_BUTTON_COUNT> mMouseButtonsReleased{};

        Math::Vector2f mMousePosition{};
        Math::Vector2f mMouseDelta{};
        Math::Vector2f mWheelDelta{};
        bool mIsMouseInWindow{false};
    };
}
