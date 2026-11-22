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
    /// @brief Rectangle that follows the mouse; a screen can have one.
    class Cursor final : public Rect {
    public:
        /// @brief Creates a cursor.
        /// @param input Size and colors of the cursor.
        explicit Cursor(const RectInput& input) : Rect(input) {}

        /// @brief Sets the texture of the cursor.
        /// @param name Name of a texture in the texture cache.
        void SetTexture(std::string name) { mTextureName = std::move(name); }
        /// @return The texture name.
        [[nodiscard]] const std::string& GetTexture() const { return mTextureName; }

    protected:
        void OnProcessInput(const Input::InputState &state) override;

    private:
        std::string mTextureName;
    };
}
