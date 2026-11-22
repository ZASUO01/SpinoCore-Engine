// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/Actors/Actor.h"
#include <algorithm>
#include <utility>
#include "SpinoCore/Core/Actors/Components/Component.h"
#include "SpinoCore/Math/MatrixFactory.h"

namespace SpinoCore::Core::Actors {
    Actor::Actor(const ActorInput &input)
    :mAnchor(input.anchor)
    ,mPosition(input.position)
    ,mScale(input.scale)
    ,mRotation(input.rotation.Normalized())
    {}

    Actor::~Actor() {
        SetParent(nullptr);
        for (Actor* child : mChildren) {
            child->mParent = nullptr;
            child->SetTransformDirty();
        }
    }

    void Actor::Update(const float deltaTime) {
        OnUpdate(deltaTime);

        for (const auto& component : mComponents) {
            if (component->IsEnabled()) {
                component->Update(deltaTime);
            }
        }
    }

    void Actor::ProcessInput(const Input::InputState &inputState) {
        OnProcessInput(inputState);
    }

    void Actor::GetDrawCommands(Rendering::ModelDrawList &drawList) const {
        for (const auto& component : mComponents) {
            if (component->IsEnabled()) {
                component->GetDrawCommand(drawList);
            }
        }
    }

    Math::Vector3f Actor::GetForward() {
        return GetWorldTransform().TransformVector(Math::Vector3f::NegUnitZ()).Normalized();
    }

    const Math::Matrix4f &Actor::GetWorldTransform() {
        if (mIsTransformDirty) {
            const Math::Matrix4f local = Math::MatrixFactory::CreateTransform(mPosition + mAnchor, mRotation, mScale);
            mWorldTransform = mParent ? mParent->GetWorldTransform() * local : local;
            mIsTransformDirty = false;
        }
        return mWorldTransform;
    }

    void Actor::SetParent(Actor* parent) {
        if (parent == mParent || parent == this || IsAncestorOf(parent)) return;

        if (mParent) std::erase(mParent->mChildren, this);

        mParent = parent;
        if (mParent) mParent->mChildren.push_back(this);

        SetTransformDirty();
    }

    void Actor::AddComponent(std::unique_ptr<Components::Component> component) {
        mComponents.emplace_back(std::move(component));

        std::ranges::stable_sort(mComponents, [](const auto& a, const auto& b) {
            return a->GetUpdateOrder() < b->GetUpdateOrder();
        });
    }

    void Actor::SetTransformDirty() {
        if (mIsTransformDirty) return;

        mIsTransformDirty = true;
        for (Actor* child : mChildren) child->SetTransformDirty();
    }

    bool Actor::IsAncestorOf(const Actor* actor) const {
        for (const Actor* current = actor ? actor->mParent : nullptr; current; current = current->mParent) {
            if (current == this) return true;
        }
        return false;
    }
}
