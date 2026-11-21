// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <string>
#include <utility>
#include "Rect.h"

namespace SpinoCore::Core::Input {
    class InputState;
}

namespace SpinoCore::Core::UI::Elements {
    class Cursor final : public Rect {
    public:
        explicit Cursor(const RectInput& input) : Rect(input) {}

        void SetTexture(std::string name) { mTextureName = std::move(name); }
        [[nodiscard]] const std::string& GetTexture() const { return mTextureName; }

    protected:
        void OnProcessInput(const Input::InputState &state) override;

    private:
        std::string mTextureName;
    };
}
