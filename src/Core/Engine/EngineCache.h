// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "Graphics/Resources/Texture.h"
#include "Assets/AssetCache.h"
#include "Graphics/Resources/Font.h"
#include "Graphics/Resources/Model.h"

namespace SpinoCore::Core::Engine {
    struct EngineCache final {
        Assets::AssetCache<Graphics::Resources::Texture> textureCache{};
        Assets::AssetCache<Graphics::Resources::Font> fontCache{};
        Assets::AssetCache<Graphics::Resources::Model> modelCache{};
    };
}
