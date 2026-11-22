// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include "ModelData.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Graphics::Geometry {
    class VertexArray;
}

namespace SpinoCore::Graphics::Resources {
    class Model final {
    public:
        struct Material final {
            Math::Vector4f baseColor{Math::Vector4f::One()};
            std::string textureId{};
        };

        struct Mesh final {
            std::unique_ptr<Geometry::VertexArray> vertices;
            std::size_t material{0};
            bool skinned{false};
        };

        Model();
        Model(const ModelData& data, std::vector<Material> materials);
        ~Model();

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;
        Model(Model&&) = delete;
        Model& operator=(Model&&) = delete;

        [[nodiscard]] const std::vector<Mesh>& GetMeshes() const noexcept { return mMeshes; }
        [[nodiscard]] const std::vector<Material>& GetMaterials() const noexcept { return mMaterials; }
        [[nodiscard]] const Skeleton& GetSkeleton() const noexcept { return mSkeleton; }
        [[nodiscard]] const std::vector<AnimationClip>& GetClips() const noexcept { return mClips; }
        [[nodiscard]] const AnimationClip* FindClip(std::string_view name) const noexcept;
        [[nodiscard]] bool HasSkeleton() const noexcept { return !mSkeleton.bones.empty(); }

    private:
        void Build(const ModelData& data);

        std::vector<Mesh> mMeshes;
        std::vector<Material> mMaterials;
        Skeleton mSkeleton;
        std::vector<AnimationClip> mClips;
    };
}
