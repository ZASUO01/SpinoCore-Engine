// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
#include "Assets/AssetCache.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Graphics::Resources {
    class Font;
    class Texture;
}

namespace SpinoCore::Core::Rendering {
    inline constexpr std::uint32_t UIKeyTextureBits = 16U;
    inline constexpr std::uint32_t UIKeyDrawOrderBits = 32U;
    inline constexpr std::uint32_t UIKeyLayerShift = UIKeyDrawOrderBits + UIKeyTextureBits;
    inline constexpr int UIKeyMaxLayer = 0xFFFF;
    inline constexpr std::uint32_t UIKeyTextureMask = (1U << UIKeyTextureBits) - 1U;

    [[nodiscard]] constexpr std::uint64_t MakeUIDrawKey(const int drawLayer, const int drawOrder, const std::uint32_t textureId) noexcept {
        const auto layer = static_cast<std::uint64_t>(std::clamp(drawLayer, 0, UIKeyMaxLayer));
        const auto order = static_cast<std::uint64_t>(static_cast<std::uint32_t>(drawOrder));
        const auto texture = static_cast<std::uint64_t>(textureId & UIKeyTextureMask);
        return layer << UIKeyLayerShift | order << UIKeyTextureBits | texture;
    }

    struct UIDrawCommand {
        uint64_t key{0};
        Math::Matrix4f transform{Math::Matrix4f::Identity()};
        Math::Vector4f textureRect{0.0f, 0.0f, 1.0f, 1.0f};
        Math::Vector4f color{1.0f, 1.0f, 1.0f, 1.0f};
        Math::Vector2f size{20.0f, 20.0f};
        float borderRadius{0.0f};
        std::uint32_t textureId{0};
        float textureMix{0.0f};
        float opacity{1.0f};
        bool tintTexture{false};
    };

    struct UIDrawList {
        std::vector<UIDrawCommand> commands;
        const Assets::AssetCache<Graphics::Resources::Texture>* textures{nullptr};
        const Assets::AssetCache<Graphics::Resources::Font>* fonts{nullptr};
    };
}
