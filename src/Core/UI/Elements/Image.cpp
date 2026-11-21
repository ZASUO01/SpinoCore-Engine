// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/UI/Elements/Image.h"
#include <cstdint>
#include "Core/Rendering/Drawer/DrawCommands.h"
#include "Graphics/Resources/Texture.h"
#include "SpinoCore/Logs/Logger.h"

namespace SpinoCore::Core::UI::Elements {
    void Image::SetDrawCommand(Rendering::UIDrawList &drawList, const int drawLayer) {
        std::uint32_t textureId = 0;
        if (const auto* textures = drawList.textures) {
            if (textures->Contains(mTextureName)) {
                textureId = textures->Get(mTextureName).GetID();
            } else if (textures->HasDefault()) {
                textureId = textures->GetDefault().GetID();
                if (!mWarnedMissingTexture) {
                    Logs::Logger::Warn(R"([UI] Texture "{}" is not cached (yet); the default texture is drawn instead.)", mTextureName);
                    mWarnedMissingTexture = true;
                }
            }
        }

        const std::uint64_t key = Rendering::MakeUIDrawKey(drawLayer, GetDrawOrder(), textureId);

        const float worldScale = GetWorldScale();
        const Math::Vector2f size = GetSize();

        drawList.commands.push_back(Rendering::UIDrawCommand{
            .key = key,
            .transform = GetTransform(),
            .color = GetBackgroundColor(),
            .size = {size.x * worldScale, size.y * worldScale},
            .borderRadius = GetBorderRadius(),
            .textureId = textureId,
            .textureMix = textureId != 0 ? 1.0f : 0.0f,
            .opacity = GetOpacity(),
        });
    }
}
