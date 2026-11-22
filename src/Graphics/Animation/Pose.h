// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <vector>
#include "Graphics/Resources/ModelData.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Quaternion.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Graphics::Animation {
    [[nodiscard]] Math::Vector3f Sample(const Resources::Keyframes<Math::Vector3f>& keyframes, float time, const Math::Vector3f& fallback);
    [[nodiscard]] Math::Quaternionf Sample(const Resources::Keyframes<Math::Quaternionf>& keyframes, float time, const Math::Quaternionf& fallback);

    void ComputeBindPose(const Resources::Skeleton& skeleton, std::vector<Math::Matrix4f>& boneMatrices);
    void ComputePose(const Resources::Skeleton& skeleton, const Resources::AnimationClip& clip, float time,
                     std::vector<Math::Matrix4f>& boneMatrices);
}
