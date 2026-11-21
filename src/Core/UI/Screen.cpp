// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/UI/Screen.h"
#include <algorithm>
#include <stdexcept>
#include <utility>

namespace SpinoCore::Core::UI {
    Elements::Element &Screen::AddElement(std::unique_ptr<Elements::Element> element) {
        if (!element) throw std::invalid_argument("[UI] Element is null.");
        mElements.push_back(std::move(element));
        return *mElements.back();
    }

    std::unique_ptr<Elements::Element> Screen::RemoveElement(Elements::Element *element) {
        const auto it = std::ranges::find_if(mElements, [element](const auto& ptr) {
            return ptr.get() == element;
        });

        if (it != mElements.end()) {
            auto ret = std::move(*it);
            mElements.erase(it);
            return ret;
        }
        return nullptr;
    }

    void Screen::ProcessInputs(const Input::InputState &state) const {
        for (const auto& element : mElements) {
            if (!element->IsEnabled()) continue;
            element->ProcessInput(state);
        }
    }

    void Screen::GetDrawCommands(ManagerKey, Rendering::UIDrawList &drawList, const int drawLayer) const {
        for (const auto& element : mElements) {
            if (!element->IsVisible()) continue;
            element->GetDrawCommand(drawList, drawLayer);
        }
    }
}
