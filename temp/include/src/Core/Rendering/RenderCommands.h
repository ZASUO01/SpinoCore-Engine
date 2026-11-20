// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <vector>

#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::Rendering {
    struct UICommand {
        std::int32_t drawOrder{0};
        Math::Matrix4f transform{Math::Matrix4f::Identity()};
        Math::Vector4f textureRect{0.0f, 0.0f, 1.0f, 1.0f};
        Math::Vector4f color{1.0f, 1.0f, 1.0f, 1.0f};
        Math::Vector2f size{20.0f, 20.0f};
        float borderRadius{0.0f};
    };
    struct UIDrawList {
        std::vector<UICommand> commands;
    };

    inline std::uint64_t GenerateRenderKey(const std::uint32_t shaderID, const std::uint32_t vaID, const std::uint32_t textureID, const std::uint32_t specialOrder = 0) {
        std::uint64_t renderKey = 0;

        renderKey |= static_cast<std::uint64_t>(shaderID & 0xFFFF) << 48;
        renderKey |= static_cast<std::uint64_t>(vaID & 0xFFFF) << 32;
        renderKey |= static_cast<std::uint64_t>(textureID & 0xFFFF) << 16;
        renderKey |= static_cast<std::uint64_t>(specialOrder & 0xFFFF);

        return renderKey;
    }
}
