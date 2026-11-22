// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#define CGLTF_IMPLEMENTATION
#include "ModelHelpers.h"
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <format>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
#include <cgltf/cgltf.h>
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/MatrixFactory.h"
#include "SpinoCore/Math/Quaternion.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Utils {
    namespace {
        namespace Res = Graphics::Resources;

        struct DataDeleter final {
            void operator()(cgltf_data* data) const noexcept { cgltf_free(data); }
        };

        using DataPointer = std::unique_ptr<cgltf_data, DataDeleter>;
        using BoneMap = std::unordered_map<const cgltf_node*, std::size_t>;

        [[noreturn]] void Fail(const std::filesystem::path& modelPath, const std::string_view reason) {
            throw std::runtime_error(std::format("[MODEL] {}: {}", modelPath.string(), reason));
        }

        [[nodiscard]] std::filesystem::path PathFromUTF8(const std::string& text) {
            return {std::u8string_view(reinterpret_cast<const char8_t*>(text.data()), text.size())};
        }

        [[nodiscard]] Math::Matrix4f ToMatrix(const cgltf_float (&values)[16]) {
            return Math::Matrix4f(values);
        }

        [[nodiscard]] Math::Matrix4f WorldOf(const cgltf_node* node) {
            cgltf_float values[16];
            cgltf_node_transform_world(node, values);
            return ToMatrix(values);
        }

        void ReadRestPose(const cgltf_node& node, Res::Bone& bone) {
            if (node.has_matrix) {
                const Math::Matrix4f matrix = ToMatrix(node.matrix);
                bone.translation = Math::MatrixFactory::GetTranslation(matrix);
                bone.scale = Math::MatrixFactory::GetScale(matrix);
                const auto column = [&matrix](const std::size_t col, const float scale) {
                    return Math::Vector3f{matrix(0, col) / scale, matrix(1, col) / scale, matrix(2, col) / scale};
                };
                bone.rotation = Math::Quaternionf::FromAxes(column(0, bone.scale.x), column(1, bone.scale.y), column(2, bone.scale.z)).Normalized();
                return;
            }
            if (node.has_translation) bone.translation = {node.translation[0], node.translation[1], node.translation[2]};
            if (node.has_rotation) bone.rotation = Math::Quaternionf{node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]}.Normalized();
            if (node.has_scale) bone.scale = {node.scale[0], node.scale[1], node.scale[2]};
        }

        [[nodiscard]] std::vector<std::uint8_t> DecodeDataURI(const cgltf_options& options, const std::string_view uri,
                                                              const std::filesystem::path& modelPath) {
            const auto comma = uri.find(',');
            if (comma == std::string_view::npos || uri.substr(0, comma).find(";base64") == std::string_view::npos) {
                Fail(modelPath, "only base64 data URIs are supported for images");
            }

            const std::string base64(uri.substr(comma + 1));
            const auto padding = static_cast<std::size_t>(std::ranges::count(base64, '='));
            const std::size_t size = base64.size() / 4 * 3 - padding;

            void* decoded = nullptr;
            if (size == 0 || cgltf_load_buffer_base64(&options, size, base64.c_str(), &decoded) != cgltf_result_success) {
                Fail(modelPath, "could not decode an embedded image");
            }

            const auto* bytes = static_cast<const std::uint8_t*>(decoded);
            std::vector result(bytes, bytes + size);
            std::free(decoded);
            return result;
        }

        void ReadImage(const cgltf_options& options, const cgltf_image& image, Res::MaterialData& material,
                       const std::filesystem::path& modelPath) {
            if (image.buffer_view) {
                const std::uint8_t* bytes = cgltf_buffer_view_data(image.buffer_view);
                if (!bytes) Fail(modelPath, "an embedded image has no data");
                material.embeddedTexture.assign(bytes, bytes + image.buffer_view->size);
                return;
            }
            if (!image.uri) return;

            const std::string_view uri{image.uri};
            if (uri.starts_with("data:")) {
                material.embeddedTexture = DecodeDataURI(options, uri, modelPath);
                return;
            }

            std::string decoded{uri};
            decoded.resize(cgltf_decode_uri(decoded.data()));
            const std::filesystem::path path = PathFromUTF8(decoded);
            if (path.is_absolute()) Fail(modelPath, "image paths must be relative to the model file");
            material.texturePath = path;
        }

        void ReadMaterials(const cgltf_options& options, const cgltf_data& data, Res::ModelData& model,
                           const std::filesystem::path& modelPath) {
            model.materials.reserve(data.materials_count + 1);
            for (std::size_t index = 0; index < data.materials_count; ++index) {
                const cgltf_material& source = data.materials[index];
                Res::MaterialData material{};
                if (source.has_pbr_metallic_roughness) {
                    const auto& pbr = source.pbr_metallic_roughness;
                    material.baseColor = {pbr.base_color_factor[0], pbr.base_color_factor[1], pbr.base_color_factor[2], pbr.base_color_factor[3]};
                    if (pbr.base_color_texture.texture && pbr.base_color_texture.texture->image) {
                        ReadImage(options, *pbr.base_color_texture.texture->image, material, modelPath);
                    }
                }
                model.materials.push_back(std::move(material));
            }
            model.materials.emplace_back();
        }

        [[nodiscard]] std::size_t DepthOf(const Res::Skeleton& skeleton, std::size_t bone) {
            std::size_t depth = 0;
            while (skeleton.bones[bone].parent >= 0) {
                bone = static_cast<std::size_t>(skeleton.bones[bone].parent);
                ++depth;
            }
            return depth;
        }

        [[nodiscard]] BoneMap ReadSkeleton(const cgltf_data& data, Res::ModelData& model, const std::filesystem::path& modelPath) {
            BoneMap bones;
            if (data.skins_count == 0) return bones;

            const cgltf_skin& skin = data.skins[0];
            if (skin.joints_count > Res::MaxBones) {
                Fail(modelPath, std::format("the skin has {} joints, but at most {} are supported", skin.joints_count, Res::MaxBones));
            }

            for (std::size_t index = 0; index < skin.joints_count; ++index) bones.emplace(skin.joints[index], index);

            auto& skeleton = model.skeleton;
            skeleton.bones.resize(skin.joints_count);
            for (std::size_t index = 0; index < skin.joints_count; ++index) {
                const cgltf_node* node = skin.joints[index];
                Res::Bone& bone = skeleton.bones[index];
                bone.name = node->name ? node->name : std::format("bone_{}", index);
                ReadRestPose(*node, bone);

                if (skin.inverse_bind_matrices) {
                    cgltf_float values[16];
                    if (!cgltf_accessor_read_float(skin.inverse_bind_matrices, index, values, 16)) {
                        Fail(modelPath, "could not read the inverse bind matrices");
                    }
                    bone.inverseBind = ToMatrix(values);
                }

                if (const auto parent = node->parent ? bones.find(node->parent) : bones.end(); parent != bones.end()) {
                    bone.parent = static_cast<int>(parent->second);
                } else if (node->parent) {
                    bone.rootOffset = WorldOf(node->parent);
                }
            }

            skeleton.evaluationOrder.resize(skeleton.bones.size());
            std::iota(skeleton.evaluationOrder.begin(), skeleton.evaluationOrder.end(), std::size_t{0});
            std::ranges::stable_sort(skeleton.evaluationOrder, {}, [&skeleton](const std::size_t bone) { return DepthOf(skeleton, bone); });
            return bones;
        }

        [[nodiscard]] const cgltf_accessor* FindAttribute(const cgltf_primitive& primitive, const cgltf_attribute_type type) {
            for (std::size_t index = 0; index < primitive.attributes_count; ++index) {
                if (const cgltf_attribute& attribute = primitive.attributes[index]; attribute.type == type && attribute.index == 0) return attribute.data;
            }
            return nullptr;
        }

        void ReadVertices(const cgltf_primitive& primitive, const bool skinned, const std::size_t boneCount,
                          Res::MeshData& mesh, const std::filesystem::path& modelPath) {
            const cgltf_accessor* positions = FindAttribute(primitive, cgltf_attribute_type_position);
            if (!positions) Fail(modelPath, "a primitive has no POSITION attribute");

            const cgltf_accessor* normals = FindAttribute(primitive, cgltf_attribute_type_normal);
            const cgltf_accessor* uvs = FindAttribute(primitive, cgltf_attribute_type_texcoord);
            const cgltf_accessor* joints = skinned ? FindAttribute(primitive, cgltf_attribute_type_joints) : nullptr;
            const cgltf_accessor* weights = skinned ? FindAttribute(primitive, cgltf_attribute_type_weights) : nullptr;
            mesh.skinned = joints && weights;

            mesh.vertices.resize(positions->count);
            for (std::size_t index = 0; index < positions->count; ++index) {
                Res::ModelVertex& vertex = mesh.vertices[index];
                cgltf_accessor_read_float(positions, index, vertex.position.Data(), 3);
                if (normals) cgltf_accessor_read_float(normals, index, vertex.normal.Data(), 3);
                if (uvs) cgltf_accessor_read_float(uvs, index, vertex.uv.Data(), 2);
                if (!mesh.skinned) continue;

                cgltf_uint jointIndices[4]{};
                cgltf_accessor_read_uint(joints, index, jointIndices, 4);
                for (std::size_t slot = 0; slot < 4; ++slot) {
                    if (jointIndices[slot] >= boneCount) Fail(modelPath, std::format("vertex {} uses joint {}, but the skin has {}", index, jointIndices[slot], boneCount));
                    vertex.joints[slot] = static_cast<std::uint8_t>(jointIndices[slot]);
                }

                cgltf_accessor_read_float(weights, index, vertex.weights.Data(), 4);
                if (const float total = vertex.weights.x + vertex.weights.y + vertex.weights.z + vertex.weights.w; total > 0.0f) {
                    vertex.weights = vertex.weights * (1.0f / total);
                }
            }
        }

        void ReadIndices(const cgltf_primitive& primitive, Res::MeshData& mesh) {
            if (!primitive.indices) {
                mesh.indices.resize(mesh.vertices.size());
                std::iota(mesh.indices.begin(), mesh.indices.end(), std::uint32_t{0});
                return;
            }

            mesh.indices.resize(primitive.indices->count);
            for (std::size_t index = 0; index < primitive.indices->count; ++index) {
                mesh.indices[index] = static_cast<std::uint32_t>(cgltf_accessor_read_index(primitive.indices, index));
            }
        }

        void BakeTransform(Res::MeshData& mesh, const Math::Matrix4f& world) {
            for (auto& vertex : mesh.vertices) {
                vertex.position = world.TransformPoint(vertex.position);
                if (const Math::Vector3f normal = world.TransformVector(vertex.normal); normal.LengthSq() > 0.0f) {
                    vertex.normal = normal.Normalized();
                }
            }
        }

        [[nodiscard]] bool SharesJoints(const cgltf_skin* skin, const cgltf_skin* reference) {
            if (!skin || !reference) return false;
            if (skin == reference) return true;
            return std::equal(skin->joints, skin->joints + skin->joints_count, reference->joints, reference->joints + reference->joints_count);
        }

        [[nodiscard]] const cgltf_node* NearestJointAbove(const cgltf_node& node, const BoneMap& bones) {
            for (const cgltf_node* current = node.parent; current; current = current->parent) {
                if (bones.contains(current)) return current;
            }
            return nullptr;
        }

        void AttachToBone(Res::MeshData& mesh, const std::size_t bone, const Res::Bone& boneData, const cgltf_node* joint,
                          const Math::Matrix4f& meshWorld) {
            const Math::Matrix4f bindSpace = boneData.inverseBind.GetInverse() * WorldOf(joint).GetInverse() * meshWorld;
            BakeTransform(mesh, bindSpace);
            for (auto& vertex : mesh.vertices) {
                vertex.joints = {static_cast<std::uint8_t>(bone), 0, 0, 0};
                vertex.weights = {1.0f, 0.0f, 0.0f, 0.0f};
            }
            mesh.skinned = true;
        }

        void ReadMeshes(const cgltf_data& data, const BoneMap& bones, Res::ModelData& model, const std::filesystem::path& modelPath) {
            const cgltf_skin* skin = data.skins_count > 0 ? &data.skins[0] : nullptr;
            const std::size_t defaultMaterial = model.materials.size() - 1;

            for (std::size_t nodeIndex = 0; nodeIndex < data.nodes_count; ++nodeIndex) {
                const cgltf_node& node = data.nodes[nodeIndex];
                if (!node.mesh) continue;

                const bool skinned = SharesJoints(node.skin, skin);
                const cgltf_node* joint = skinned ? nullptr : NearestJointAbove(node, bones);
                const Math::Matrix4f world = WorldOf(&node);

                for (std::size_t primitiveIndex = 0; primitiveIndex < node.mesh->primitives_count; ++primitiveIndex) {
                    const cgltf_primitive& primitive = node.mesh->primitives[primitiveIndex];
                    if (primitive.type != cgltf_primitive_type_triangles) continue;

                    Res::MeshData mesh{};
                    mesh.material = primitive.material ? cgltf_material_index(&data, primitive.material) : defaultMaterial;
                    ReadVertices(primitive, skinned, model.skeleton.bones.size(), mesh, modelPath);
                    ReadIndices(primitive, mesh);
                    if (!mesh.skinned && joint) {
                        const std::size_t bone = bones.at(joint);
                        AttachToBone(mesh, bone, model.skeleton.bones[bone], joint, world);
                    } else if (!mesh.skinned) {
                        BakeTransform(mesh, world);
                    }
                    model.meshes.push_back(std::move(mesh));
                }
            }

            if (model.meshes.empty()) Fail(modelPath, "the file has no triangle meshes");
        }

        [[nodiscard]] std::vector<float> ReadFloats(const cgltf_accessor& accessor, const std::size_t components) {
            std::vector<float> values(accessor.count * components);
            for (std::size_t index = 0; index < accessor.count; ++index) {
                cgltf_accessor_read_float(&accessor, index, values.data() + index * components, components);
            }
            return values;
        }

        template <typename T, typename Make>
        void ReadKeyframes(const cgltf_animation_sampler& sampler, const std::size_t components, Res::Keyframes<T>& keyframes, Make make) {
            keyframes.times = ReadFloats(*sampler.input, 1);
            keyframes.interpolation = sampler.interpolation == cgltf_interpolation_type_step ? Res::Interpolation::Step : Res::Interpolation::Linear;

            const std::vector<float> raw = ReadFloats(*sampler.output, components);
            const bool cubic = sampler.interpolation == cgltf_interpolation_type_cubic_spline;
            const std::size_t stride = cubic ? 3 : 1;
            const std::size_t offset = cubic ? 1 : 0;

            keyframes.values.clear();
            keyframes.values.reserve(keyframes.times.size());
            for (std::size_t key = 0; key < keyframes.times.size(); ++key) {
                keyframes.values.push_back(make(raw.data() + (key * stride + offset) * components));
            }
        }

        template <typename T>
        [[nodiscard]] const float* LastTime(const Res::Keyframes<T>& keyframes) {
            return keyframes.IsEmpty() ? nullptr : &keyframes.times.back();
        }

        void ReadAnimations(const cgltf_data& data, const BoneMap& bones, Res::ModelData& model) {
            for (std::size_t animationIndex = 0; animationIndex < data.animations_count; ++animationIndex) {
                const cgltf_animation& animation = data.animations[animationIndex];
                Res::AnimationClip clip{};
                clip.name = animation.name ? animation.name : std::format("animation_{}", animationIndex);

                for (std::size_t channelIndex = 0; channelIndex < animation.channels_count; ++channelIndex) {
                    const cgltf_animation_channel& channel = animation.channels[channelIndex];
                    const auto bone = bones.find(channel.target_node);
                    if (bone == bones.end() || !channel.sampler) continue;

                    auto track = std::ranges::find(clip.tracks, bone->second, &Res::BoneTrack::bone);
                    if (track == clip.tracks.end()) {
                        clip.tracks.push_back(Res::BoneTrack{.bone = bone->second, .translation = {}, .rotation = {}, .scale = {}});
                        track = std::prev(clip.tracks.end());
                    }

                    const auto vector3 = [](const float* v) { return Math::Vector3f{v[0], v[1], v[2]}; };
                    switch (channel.target_path) {
                        case cgltf_animation_path_type_translation:
                            ReadKeyframes(*channel.sampler, 3, track->translation, vector3);
                            break;
                        case cgltf_animation_path_type_rotation:
                            ReadKeyframes(*channel.sampler, 4, track->rotation, [](const float* v) { return Math::Quaternionf{v[0], v[1], v[2], v[3]}.Normalized(); });
                            break;
                        case cgltf_animation_path_type_scale:
                            ReadKeyframes(*channel.sampler, 3, track->scale, vector3);
                            break;
                        default:
                            continue;
                    }

                    for (const float* last : {LastTime(track->translation), LastTime(track->rotation), LastTime(track->scale)}) {
                        if (last) clip.duration = std::max(clip.duration, *last);
                    }
                }

                model.clips.push_back(std::move(clip));
            }
        }
    }

    Graphics::Resources::ModelData ParseModel(const std::span<const std::uint8_t> data, const std::filesystem::path& modelPath) {
        if (data.empty()) Fail(modelPath, "the file is empty");

        constexpr cgltf_options options{};
        cgltf_data* parsed = nullptr;
        if (cgltf_parse(&options, data.data(), data.size(), &parsed) != cgltf_result_success) {
            Fail(modelPath, "not a valid glTF or GLB file");
        }
        const DataPointer gltf{parsed};

        const std::string pathText = modelPath.string();
        if (cgltf_load_buffers(&options, gltf.get(), pathText.c_str()) != cgltf_result_success) {
            Fail(modelPath, "could not load the model buffers");
        }
        if (cgltf_validate(gltf.get()) != cgltf_result_success) {
            Fail(modelPath, "the glTF data is inconsistent");
        }

        Res::ModelData model{};
        ReadMaterials(options, *gltf, model, modelPath);
        const BoneMap bones = ReadSkeleton(*gltf, model, modelPath);
        ReadMeshes(*gltf, bones, model, modelPath);
        ReadAnimations(*gltf, bones, model);
        return model;
    }
}
