// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <vector>
#include "SpinoCore/Core/UI/Elements/Element.h"

namespace SpinoCore::Core::Rendering {
    class UIDrawList;
}

namespace SpinoCore::Core::UI {
    class Screen final {
    public:
        Screen() = default;
        ~Screen() = default;

        Screen(const Screen&) = delete;
        Screen& operator=(const Screen&) = delete;
        Screen(Screen&&) = delete;
        Screen& operator=(Screen&&) = delete;

        [[nodiscard]] Elements::Element& AddElement(std::unique_ptr<Elements::Element> element);
        [[nodiscard]] std::unique_ptr<Elements::Element> RemoveElement(Elements::Element* element);
        [[nodiscard]] const std::vector<std::unique_ptr<Elements::Element>>& GetElements() const { return mElements; }

        [[nodiscard]] bool IsVisible() const { return mIsVisible; }
        void SetIsVisible(const bool isVisible) { mIsVisible = isVisible; }

        class DrawKey { friend class EngineScreensManager; DrawKey() = default; };
        void GetDrawCommands(Rendering::UIDrawList& drawList, DrawKey);
    private:
        std::vector<std::unique_ptr<Elements::Element>> mElements;
        bool mIsVisible{true};
    };
}
