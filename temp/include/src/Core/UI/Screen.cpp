// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/UI/Screen.h"


namespace SpinoCore::Core::UI {
    Elements::Element& Screen::AddElement(std::unique_ptr<Elements::Element> element) {
        if (!element) throw std::runtime_error("[UI] element is null");

        mElements.push_back(std::move(element));
        return *mElements.back();
    }

     std::unique_ptr<Elements::Element> Screen::RemoveElement(Elements::Element *element) {
        const auto it = std::ranges::find_if(mElements, [&](const auto& c) { return c.get() == element; });
        if (it == mElements.end()) return nullptr;

        std::unique_ptr<Elements::Element> removed = std::move(*it);
        mElements.erase(it);
        return removed;
    }

     void Screen::GetDrawCommands(Rendering::UIDrawList &drawList, DrawKey) const {
        if (!mIsVisible) return;

         for (auto& element : mElements) {
             element->GetDrawCommand(drawList);
         }
     }
}
