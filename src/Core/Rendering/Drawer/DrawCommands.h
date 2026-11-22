// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>
#include "Assets/AssetCache.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Graphics::Resources {
    class Font;
    class Model;
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
        float borderWidth{0.0f};
        Math::Vector4f borderColor{0.0f, 0.0f, 0.0f, 0.0f};
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

    struct ModelDrawCommand {
        std::uint64_t key{0};
        const Graphics::Resources::Model* model{nullptr};
        Math::Matrix4f world{Math::Matrix4f::Identity()};
        std::span<const Math::Matrix4f> bones{};
    };

    struct DebugVertex {
        Math::Vector3f position{Math::Vector3f::Zero()};
        Math::Vector4f color{Math::Vector4f::One()};
    };

    struct DebugDrawList {
        std::vector<DebugVertex> lines;
    };

    struct ParticleInstance {
        Math::Vector3f position{Math::Vector3f::Zero()};
        float size{0.1f};
        Math::Vector4f color{Math::Vector4f::One()};
        float rotation{0.0f};
    };

    struct ParticleBatch {
        std::size_t first{0};
        std::size_t count{0};
        bool additive{true};
    };

    struct ParticleDrawList {
        std::vector<ParticleInstance> instances;
        std::vector<ParticleBatch> batches;
    };

    struct ModelDrawList {
        std::vector<ModelDrawCommand> commands;
        const Assets::AssetCache<Graphics::Resources::Texture>* textures{nullptr};
        const Assets::AssetCache<Graphics::Resources::Model>* models{nullptr};
    };
}
