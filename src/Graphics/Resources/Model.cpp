// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Model.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <format>
#include <initializer_list>
#include <span>
#include <stdexcept>
#include <utility>
#include "Graphics/Geometry/VertexArray.h"

namespace SpinoCore::Graphics::Resources {
    namespace {
        static_assert(sizeof(ModelVertex) == 52, "ModelVertex must be tightly packed for the vertex layout.");

        [[nodiscard]] MeshData MakeCube() {
            struct Face final {
                Math::Vector3f normal;
                Math::Vector3f right;
                Math::Vector3f up;
            };

            constexpr std::array<Face, 6> faces{{
                {{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
                {{-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
                {{0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
                {{0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                {{0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                {{0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            }};
            constexpr std::array<Math::Vector2f, 4> corners{{{-1.0f, -1.0f}, {1.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 1.0f}}};

            MeshData cube{};
            for (const auto& [normal, right, up] : faces) {
                const auto first = static_cast<std::uint32_t>(cube.vertices.size());
                for (const auto& corner : corners) {
                    ModelVertex vertex{};
                    vertex.position = (normal + right * corner.x + up * corner.y) * 0.5f;
                    vertex.normal = normal;
                    vertex.uv = {(corner.x + 1.0f) * 0.5f, (corner.y + 1.0f) * 0.5f};
                    cube.vertices.push_back(vertex);
                }
                for (const std::uint32_t offset : {0u, 1u, 2u, 2u, 3u, 0u}) cube.indices.push_back(first + offset);
            }
            return cube;
        }

        [[nodiscard]] std::unique_ptr<Geometry::VertexArray> Upload(const MeshData& mesh) {
            using Geometry::AttributeType;
            return std::make_unique<Geometry::VertexArray>(Geometry::VertexArray::Input{
                .vertexData = std::as_bytes(std::span(mesh.vertices)),
                .indices = mesh.indices,
                .layout = {
                    {.location = 0, .type = AttributeType::Float3},
                    {.location = 1, .type = AttributeType::Float3},
                    {.location = 2, .type = AttributeType::Float2},
                    {.location = 3, .type = AttributeType::UByte4},
                    {.location = 4, .type = AttributeType::Float4},
                },
            });
        }
    }

    Model::Model() : mMaterials{Material{}} {
        ModelData data{};
        data.meshes.push_back(MakeCube());
        Build(data);
    }

    Model::Model(const ModelData& data, std::vector<Material> materials) : mMaterials(std::move(materials)) {
        Build(data);
    }

    Model::~Model() = default;

    const AnimationClip* Model::FindClip(const std::string_view name) const noexcept {
        const auto it = std::ranges::find(mClips, name, &AnimationClip::name);
        return it != mClips.end() ? &*it : nullptr;
    }

    void Model::Build(const ModelData& data) {
        if (data.meshes.empty()) throw std::invalid_argument("[MODEL] A model needs at least one mesh.");
        if (data.skeleton.bones.size() > MaxBones) {
            throw std::invalid_argument(std::format("[MODEL] The skeleton has {} bones, but at most {} are supported.",
                                                    data.skeleton.bones.size(), MaxBones));
        }

        for (const auto& mesh : data.meshes) {
            if (mesh.material >= mMaterials.size()) {
                throw std::invalid_argument(std::format("[MODEL] A mesh uses material {}, but the model has {}.", mesh.material, mMaterials.size()));
            }
            mMeshes.push_back(Mesh{.vertices = Upload(mesh), .material = mesh.material, .skinned = mesh.skinned && !data.skeleton.bones.empty()});
        }

        mSkeleton = data.skeleton;
        mClips = data.clips;
    }
}
