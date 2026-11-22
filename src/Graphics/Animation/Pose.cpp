// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Pose.h"
#include <algorithm>
#include <cstddef>
#include <iterator>
#include "SpinoCore/Math/MatrixFactory.h"

namespace SpinoCore::Graphics::Animation {
    namespace {
        struct Segment final {
            std::size_t from{0};
            std::size_t to{0};
            float factor{0.0f};
        };

        template <typename T>
        [[nodiscard]] Segment FindSegment(const Resources::Keyframes<T>& keyframes, const float time) {
            const auto& times = keyframes.times;
            if (time <= times.front()) return {};
            if (time >= times.back()) return {.from = times.size() - 1, .to = times.size() - 1, .factor = 0.0f};

            const auto upper = std::ranges::upper_bound(times, time);
            const auto to = static_cast<std::size_t>(std::distance(times.begin(), upper));
            const std::size_t from = to - 1;
            if (keyframes.interpolation == Resources::Interpolation::Step) return {.from = from, .to = from, .factor = 0.0f};

            const float span = times[to] - times[from];
            const float factor = span > 0.0f ? (time - times[from]) / span : 0.0f;
            return {.from = from, .to = to, .factor = factor};
        }

        void ComputeWithLocals(const Resources::Skeleton& skeleton, const std::vector<Math::Matrix4f>& locals,
                               std::vector<Math::Matrix4f>& boneMatrices) {
            const std::size_t count = skeleton.bones.size();
            std::vector<Math::Matrix4f> world(count, Math::Matrix4f::Identity());
            boneMatrices.assign(count, Math::Matrix4f::Identity());

            for (const std::size_t index : skeleton.evaluationOrder) {
                const Resources::Bone& bone = skeleton.bones[index];
                const Math::Matrix4f& parentWorld = bone.parent >= 0 ? world[static_cast<std::size_t>(bone.parent)] : bone.rootOffset;
                world[index] = parentWorld * locals[index];
                boneMatrices[index] = world[index] * bone.inverseBind;
            }
        }

        [[nodiscard]] Math::Matrix4f RestLocal(const Resources::Bone& bone) {
            return Math::MatrixFactory::CreateTransform(bone.translation, bone.rotation, bone.scale);
        }
    }

    Math::Vector3f Sample(const Resources::Keyframes<Math::Vector3f>& keyframes, const float time, const Math::Vector3f& fallback) {
        if (keyframes.IsEmpty()) return fallback;
        const auto [from, to, factor] = FindSegment(keyframes, time);
        return Math::Vector3f::Lerp(keyframes.values[from], keyframes.values[to], factor);
    }

    Math::Quaternionf Sample(const Resources::Keyframes<Math::Quaternionf>& keyframes, const float time, const Math::Quaternionf& fallback) {
        if (keyframes.IsEmpty()) return fallback;
        const auto [from, to, factor] = FindSegment(keyframes, time);
        if (from == to) return keyframes.values[from].Normalized();
        return Math::Quaternionf::Slerp(keyframes.values[from], keyframes.values[to], factor).Normalized();
    }

    void ComputeBindPose(const Resources::Skeleton& skeleton, std::vector<Math::Matrix4f>& boneMatrices) {
        std::vector<Math::Matrix4f> locals;
        locals.reserve(skeleton.bones.size());
        std::ranges::transform(skeleton.bones, std::back_inserter(locals), RestLocal);
        ComputeWithLocals(skeleton, locals, boneMatrices);
    }

    void ComputePose(const Resources::Skeleton& skeleton, const Resources::AnimationClip& clip, const float time,
                     std::vector<Math::Matrix4f>& boneMatrices) {
        std::vector<Resources::Bone> posed = skeleton.bones;
        for (const auto& track : clip.tracks) {
            if (track.bone >= posed.size()) continue;
            Resources::Bone& bone = posed[track.bone];
            bone.translation = Sample(track.translation, time, bone.translation);
            bone.rotation = Sample(track.rotation, time, bone.rotation);
            bone.scale = Sample(track.scale, time, bone.scale);
        }

        std::vector<Math::Matrix4f> locals;
        locals.reserve(posed.size());
        std::ranges::transform(posed, std::back_inserter(locals), RestLocal);
        ComputeWithLocals(skeleton, locals, boneMatrices);
    }
}
