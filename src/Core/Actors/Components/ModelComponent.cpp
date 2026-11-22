// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/Actors/Components/ModelComponent.h"
#include <span>
#include <utility>
#include "Assets/AssetCache.h"
#include "Core/Rendering/Drawer/DrawCommands.h"
#include "SpinoCore/Core/Actors/Actor.h"
#include "SpinoCore/Core/Actors/Components/AnimatorComponent.h"
#include "SpinoCore/Math/Matrix.h"

namespace SpinoCore::Core::Actors::Components {
    ModelComponent::ModelComponent(Actor& actor, std::string modelId, const ComponentInput& input)
    :Component(actor, input)
    ,mModelId(std::move(modelId))
    {}

    void ModelComponent::Update(float) {}

    void ModelComponent::SetModelId(std::string modelId) {
        mModelId = std::move(modelId);
        mModel = nullptr;
        mCachedFrom = nullptr;
    }

    void ModelComponent::GetDrawCommand(Rendering::ModelDrawList& drawList) {
        ResolveModel(drawList);
        if (!mModel) return;

        Actor* actor = GetActor();
        std::span<const Math::Matrix4f> bones{};
        if (const auto* animator = actor->GetComponent<AnimatorComponent>(); animator && animator->IsEnabled()) {
            bones = animator->GetBoneMatrices();
        }

        drawList.commands.push_back(Rendering::ModelDrawCommand{
            .key = static_cast<std::uint32_t>(GetDrawOrder()),
            .model = mModel,
            .world = actor->GetWorldTransform(),
            .bones = bones,
        });
    }

    void ModelComponent::ResolveModel(const Rendering::ModelDrawList& drawList) {
        const auto* models = drawList.models;
        if (!models) {
            mModel = nullptr;
            mCachedFrom = nullptr;
            return;
        }
        if (models == mCachedFrom && models->GetVersion() == mCachedVersion) return;

        if (models->Contains(mModelId)) mModel = &models->Get(mModelId);
        else mModel = models->HasDefault() ? &models->GetDefault() : nullptr;

        mCachedFrom = models;
        mCachedVersion = models->GetVersion();
    }
}
