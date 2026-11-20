// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/UI/Elements/Rect.h"
#include <cstdint>
#include "Core/Rendering/Drawer/DrawCommands.h"

namespace SpinoCore::Core::UI::Elements {
    void Rect::SetDrawCommand(Rendering::UIDrawList &drawList, const int drawLayer){
        Math::Vector4f color = mBackgroundColor;
        color.w *= mOpacity;

        const std::uint64_t key = static_cast<std::uint64_t>(drawLayer) << 32U |
                                  static_cast<std::uint32_t>(GetDrawOrder());

        const float worldScale = GetWorldScale();
        const Math::Vector2f size = GetSize();

        const Rendering::UIDrawCommand cmd = {
            .key = key,
            .transform = GetTransform(),
            .color = color,
            .size = {size.x * worldScale, size.y * worldScale},
            .borderRadius = mBorderRadius,
        };
        drawList.commands.push_back(cmd);
    }
}
