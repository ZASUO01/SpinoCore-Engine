// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/UI/Screen.h"
#include <algorithm>
#include <stdexcept>
#include <utility>
#include "SpinoCore/Core/UI/Elements/Cursor.h"

namespace SpinoCore::Core::UI {
    Elements::Element &Screen::AddElement(std::unique_ptr<Elements::Element> element) {
        if (!element) throw std::invalid_argument("[UI] Element is null.");
        if (auto* cursor = dynamic_cast<Elements::Cursor*>(element.get())) {
            if (mCursor) throw std::invalid_argument("[UI] The screen already has a cursor.");
            mCursor = cursor;
        }
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
            if (ret.get() == mCursor) mCursor = nullptr;
            return ret;
        }
        return nullptr;
    }

    void Screen::ProcessInputs(const Input::InputState &state) const {
        if (mCursor && mCursor->IsEnabled()) mCursor->ProcessInput(state);

        const auto focused = std::ranges::find_if(mElements, [](const auto& element) {
            return element->IsEnabled() && element->HasFocus();
        });
        if (focused != mElements.end()) {
            (*focused)->ProcessInput(state);
            return;
        }

        for (const auto& element : mElements) {
            if (element.get() == mCursor || !element->IsEnabled()) continue;
            element->ProcessInput(state);
            if (element->HasFocus()) return;
        }
    }

    void Screen::GetDrawCommands(ManagerKey, Rendering::UIDrawList &drawList, const int drawLayer) const {
        for (const auto& element : mElements) {
            if (!element->IsVisible()) continue;
            element->GetDrawCommand(drawList, drawLayer);
        }
    }
}
