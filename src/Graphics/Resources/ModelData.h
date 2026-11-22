// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Quaternion.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Graphics::Resources {
    inline constexpr std::size_t MaxBones = 64;

    struct ModelVertex final {
        Math::Vector3f position{Math::Vector3f::Zero()};
        Math::Vector3f normal{Math::Vector3f::UnitY()};
        Math::Vector2f uv{Math::Vector2f::Zero()};
        std::array<std::uint8_t, 4> joints{0, 0, 0, 0};
        Math::Vector4f weights{Math::Vector4f::Zero()};
    };

    struct MeshData final {
        std::vector<ModelVertex> vertices;
        std::vector<std::uint32_t> indices;
        std::size_t material{0};
        bool skinned{false};
    };

    struct MaterialData final {
        Math::Vector4f baseColor{Math::Vector4f::One()};
        std::optional<std::filesystem::path> texturePath{};
        std::vector<std::uint8_t> embeddedTexture{};
    };

    struct Bone final {
        std::string name;
        int parent{-1};
        Math::Matrix4f inverseBind{Math::Matrix4f::Identity()};
        Math::Matrix4f rootOffset{Math::Matrix4f::Identity()};
        Math::Vector3f translation{Math::Vector3f::Zero()};
        Math::Quaternionf rotation{Math::Quaternionf::Identity()};
        Math::Vector3f scale{Math::Vector3f::One()};
    };

    struct Skeleton final {
        std::vector<Bone> bones;
        std::vector<std::size_t> evaluationOrder;
    };

    enum class Interpolation : std::uint8_t {
        Linear,
        Step
    };

    template <typename T>
    struct Keyframes final {
        std::vector<float> times;
        std::vector<T> values;
        Interpolation interpolation{Interpolation::Linear};

        [[nodiscard]] bool IsEmpty() const noexcept { return times.empty(); }
    };

    struct BoneTrack final {
        std::size_t bone{0};
        Keyframes<Math::Vector3f> translation;
        Keyframes<Math::Quaternionf> rotation;
        Keyframes<Math::Vector3f> scale;
    };

    struct AnimationClip final {
        std::string name;
        float duration{0.0f};
        std::vector<BoneTrack> tracks;
    };

    struct ModelData final {
        std::vector<MeshData> meshes;
        std::vector<MaterialData> materials;
        Skeleton skeleton;
        std::vector<AnimationClip> clips;
    };
}
