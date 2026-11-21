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

namespace SpinoCore::Core::UI {
    class Screen {
    public:
        Screen() = default;
        virtual ~Screen() = default;

        Screen(const Screen&) = delete;
        Screen& operator=(const Screen&) = delete;
        Screen(Screen&&) = delete;
        Screen& operator=(Screen&&) = delete;

        // Throws std::invalid_argument for a null element or for a second Cursor: a screen has at most one.
        [[nodiscard]] Elements::Element& AddElement(std::unique_ptr<Elements::Element> element);
        [[nodiscard]] std::unique_ptr<Elements::Element> RemoveElement(Elements::Element* element);
        [[nodiscard]] const std::vector<std::unique_ptr<Elements::Element>>& GetElements() const { return mElements; }

        [[nodiscard]] bool IsVisible() const { return mIsVisible; }
        void SetIsVisible(const bool isVisible) { mIsVisible = isVisible; }

        virtual void OnEnter() = 0;
        virtual void OnPause() = 0;
        virtual void OnResume() = 0;
        virtual void OnExit() = 0;

        class ManagerKey{ friend class EngineScreensManager; ManagerKey() = default; };
        // The cursor always gets the input. A focused element (e.g. an InputField being typed in) gets it alone;
        // otherwise every enabled element does, in order, until one of them takes the focus.
        void ProcessInputs(const Input::InputState& state) const;
        void GetDrawCommands(ManagerKey, Rendering::UIDrawList& drawList, int drawLayer) const;

    private:
        std::vector<std::unique_ptr<Elements::Element>> mElements;
        Elements::Cursor* mCursor{nullptr};
        bool mIsVisible{true};
    };
}
