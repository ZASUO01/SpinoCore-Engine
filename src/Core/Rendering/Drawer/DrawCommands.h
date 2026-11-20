// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <vector>
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::Rendering {
    struct UIDrawCommand {
        uint64_t key{0};
        Math::Matrix4f transform{Math::Matrix4f::Identity()};
        Math::Vector4f textureRect{0.0f, 0.0f, 1.0f, 1.0f};
        Math::Vector4f color{1.0f, 1.0f, 1.0f, 1.0f};
        Math::Vector2f size{20.0f, 20.0f};
        float borderRadius{0.0f};
    };
    struct UIDrawList {
        std::vector<UIDrawCommand> commands;
    };
}
