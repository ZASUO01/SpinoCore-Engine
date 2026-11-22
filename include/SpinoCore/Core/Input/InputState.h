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

/// @brief Keyboard and mouse state read by the UI, the scenes and the actors.
namespace SpinoCore::Core::Input {
    /// @brief Mouse buttons tracked by InputState.
    enum class MouseButton : std::uint8_t { Left, Middle, Right, X1, X2 };

    /// @brief Snapshot of the keyboard and the mouse.
    ///
    /// The engine keeps one state per frame for the UI and one per simulation step for scenes and actors.
    /// "Down" means held now; "pressed" and "released" mean it changed since the last frame or step.
    class InputState final {
    public:
        /// @brief Clears the per-frame changes and keeps what is held.
        void BeginFrame() noexcept;
        /// @brief Updates the state with an SDL event.
        /// @param event Keyboard, mouse, text or focus event.
        void Apply(const SDL_Event& event) noexcept;
        /// @brief Releases every held key and button, as when the window loses focus.
        void ReleaseAll() noexcept;
        /// @brief Merges a frame into this state so no change is lost between simulation steps.
        /// @param frame State of a rendered frame.
        void Accumulate(const InputState& frame);

        /// @param key SDL scancode.
        /// @return Whether the key is held.
        [[nodiscard]] bool IsKeyDown(SDL_Scancode key) const noexcept;
        /// @param key SDL scancode.
        /// @return Whether the key went down since the last frame or step.
        [[nodiscard]] bool WasKeyPressed(SDL_Scancode key) const noexcept;
        /// @param key SDL scancode.
        /// @return Whether the key went up since the last frame or step.
        [[nodiscard]] bool WasKeyReleased(SDL_Scancode key) const noexcept;
        /// @param key SDL scancode.
        /// @return Whether the key was pressed or repeated by the system, as used for text editing.
        [[nodiscard]] bool WasKeyTyped(SDL_Scancode key) const noexcept;

        /// @return The UTF-8 text typed since the last frame or step.
        [[nodiscard]] const std::string& GetTypedText() const noexcept { return mTypedText; }

        /// @param button Mouse button.
        /// @return Whether the button is held.
        [[nodiscard]] bool IsMouseButtonDown(MouseButton button) const noexcept;
        /// @param button Mouse button.
        /// @return Whether the button went down since the last frame or step.
        [[nodiscard]] bool WasMouseButtonPressed(MouseButton button) const noexcept;
        /// @param button Mouse button.
        /// @return Whether the button went up since the last frame or step.
        [[nodiscard]] bool WasMouseButtonReleased(MouseButton button) const noexcept;

        /// @return The mouse position in window pixels, from the top-left corner.
        [[nodiscard]] const Math::Vector2f& GetMousePosition() const noexcept { return mMousePosition; }
        /// @return The mouse movement since the last frame or step.
        [[nodiscard]] const Math::Vector2f& GetMouseDelta() const noexcept { return mMouseDelta; }
        /// @return The wheel movement since the last frame or step.
        [[nodiscard]] const Math::Vector2f& GetWheelDelta() const noexcept { return mWheelDelta; }
        /// @return Whether the mouse is over the window.
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
