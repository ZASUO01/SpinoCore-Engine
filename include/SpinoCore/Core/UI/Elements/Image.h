// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <string>
#include <utility>
#include "Rect.h"

namespace SpinoCore::Core::Rendering {
    struct UIDrawList;
}

namespace SpinoCore::Core::UI::Elements {
    /// @brief Settings of an image.
    struct ImageInput final : RectInput {
        /// Name of a texture in the texture cache; a missing texture draws the default one.
        std::string name{"default"};
    };

    /// @brief Rectangle filled with a texture.
    class Image final : public Rect {
    public:
        /// @brief Creates an image.
        /// @param input Rectangle and texture settings.
        explicit Image(const ImageInput& input) : Rect(input), mTextureName(input.name) {}

        /// @brief Changes the texture.
        /// @param name Name of a texture in the texture cache.
        void SetTexture(std::string name) { mTextureName = std::move(name); mIsCachedTexture = false; mWarnedMissingTexture = false; }
        /// @return The texture name.
        [[nodiscard]] const std::string& GetTexture() const { return mTextureName; }

    protected:
        void SetDrawCommand(Rendering::UIDrawList &drawList, int drawLayer) override;

    private:
        std::string mTextureName;
        const void* mCachedFrom{nullptr};
        std::uint64_t mCachedVersion{0};
        std::uint32_t mCachedTextureId{0};
        bool mIsCachedTexture{false};
        bool mWarnedMissingTexture{false};
    };
}
