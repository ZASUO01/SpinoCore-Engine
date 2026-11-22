// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include <vector>
#include "SpinoCore/Core/UI/Elements/Element.h"

namespace SpinoCore::Core::Rendering {
    struct UIDrawList;
}

namespace SpinoCore::Core::Input {
    class InputState;
}

namespace SpinoCore::Core::UI::Elements {
    class Cursor;
}

/// @brief User interface: screens and the elements drawn on them.
namespace SpinoCore::Core::UI {
    /// @brief A layer of UI elements, such as a menu or a HUD.
    ///
    /// Screens are stacked by the ScreensManager. Only the top screen receives input;
    /// every visible screen is drawn.
    class Screen {
    public:
        Screen() = default;
        virtual ~Screen() = default;

        Screen(const Screen&) = delete;
        Screen& operator=(const Screen&) = delete;
        Screen(Screen&&) = delete;
        Screen& operator=(Screen&&) = delete;

        /// @brief Adds a root element; a Cursor element is drawn above the others and gets input first.
        /// @param element Element to own.
        /// @return The added element.
        /// @throws std::invalid_argument When the element is null or is a second cursor.
        [[nodiscard]] Elements::Element& AddElement(std::unique_ptr<Elements::Element> element);
        /// @brief Removes a root element.
        /// @param element Element to remove.
        /// @return The element, or nullptr when it is not a root element of this screen.
        [[nodiscard]] std::unique_ptr<Elements::Element> RemoveElement(Elements::Element* element);
        /// @return The root elements.
        [[nodiscard]] const std::vector<std::unique_ptr<Elements::Element>>& GetElements() const { return mElements; }

        /// @return Whether the screen is drawn.
        [[nodiscard]] bool IsVisible() const { return mIsVisible; }
        /// @brief Shows or hides the screen.
        /// @param isVisible true to draw it.
        void SetIsVisible(const bool isVisible) { mIsVisible = isVisible; }

        /// @brief Called when the screen is pushed.
        virtual void OnEnter() = 0;
        /// @brief Called when another screen is pushed on top of this one.
        virtual void OnPause() = 0;
        /// @brief Called when the screen on top of this one is popped.
        virtual void OnResume() = 0;
        /// @brief Called when the screen is popped.
        virtual void OnExit() = 0;

        /// @brief Passkey that only the engine screens manager can build.
        class ManagerKey{ friend class EngineScreensManager; ManagerKey() = default; };
        /// @brief Sends the frame input to the cursor and then to the focused element, or to every enabled element.
        /// @param state Input of the current frame.
        void ProcessInputs(const Input::InputState& state) const;
        /// @brief Adds the visible elements to the UI draw list.
        /// @param drawList List that receives the commands.
        /// @param drawLayer Position of the screen in the stack.
        void GetDrawCommands(ManagerKey, Rendering::UIDrawList& drawList, int drawLayer) const;

    private:
        std::vector<std::unique_ptr<Elements::Element>> mElements;
        Elements::Cursor* mCursor{nullptr};
        bool mIsVisible{true};
    };
}
