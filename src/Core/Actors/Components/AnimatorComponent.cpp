// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/Actors/Components/AnimatorComponent.h"
#include <cmath>
#include <utility>
#include "Graphics/Animation/Pose.h"
#include "Graphics/Resources/Model.h"
#include "Graphics/Resources/ModelData.h"
#include "SpinoCore/Core/Actors/Actor.h"
#include "SpinoCore/Core/Actors/Components/ModelComponent.h"
#include "SpinoCore/Logs/Logger.h"

namespace SpinoCore::Core::Actors::Components {
    AnimatorComponent::AnimatorComponent(Actor& actor, const ComponentInput& input)
    :Component(actor, input)
    {}

    void AnimatorComponent::Play(std::string clipName, const bool loop) {
        mClipName = std::move(clipName);
        mTime = 0.0f;
        mIsLooping = loop;
        mIsPlaying = true;
    }

    void AnimatorComponent::Stop() {
        mIsPlaying = false;
    }

    void AnimatorComponent::GetDrawCommand(Rendering::ModelDrawList&) {}

    void AnimatorComponent::Update(const float deltaTime) {
        const auto* modelComponent = GetActor()->GetComponent<ModelComponent>();
        const Graphics::Resources::Model* model = modelComponent ? modelComponent->GetModel() : nullptr;
        if (!model || !model->HasSkeleton()) {
            mBoneMatrices.clear();
            return;
        }

        const Graphics::Resources::AnimationClip* clip = mClipName.empty() ? nullptr : model->FindClip(mClipName);
        if (!clip) {
            if (!mClipName.empty()) {
                Logs::Logger::Warn(R"([ANIMATOR] Model "{}" has no clip named "{}".)", modelComponent->GetModelId(), mClipName);
                mClipName.clear();
                mIsPlaying = false;
            }
            Graphics::Animation::ComputeBindPose(model->GetSkeleton(), mBoneMatrices);
            return;
        }

        if (mIsPlaying) Advance(deltaTime, clip->duration);
        Graphics::Animation::ComputePose(model->GetSkeleton(), *clip, mTime, mBoneMatrices);
    }

    void AnimatorComponent::Advance(const float deltaTime, const float duration) {
        if (duration <= 0.0f) {
            mTime = 0.0f;
            return;
        }

        mTime += deltaTime;
        if (mIsLooping) {
            mTime = std::fmod(mTime, duration);
        } else if (mTime >= duration) {
            mTime = duration;
            mIsPlaying = false;
        }
    }
}
