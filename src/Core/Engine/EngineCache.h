// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <optional>
#include "Graphics/Resources/Texture.h"
#include "Assets/AssetCache.h"

namespace SpinoCore::Core::Engine {
    struct EngineCache final {
        std::optional<Assets::AssetCache<Graphics::Resources::Texture>> textureCache{};
    };
}
