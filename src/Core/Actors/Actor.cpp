// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/Actors/Actor.h"
#include <algorithm>
#include <utility>
#include "Core/Actors/ComponentListener.h"
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
        if (mComponentListener) {
            for (const auto& component : mComponents) mComponentListener->OnComponentDetached(*component);
        }
        SetParent(nullptr);
        for (Actor* child : mChildren) {
            child->mParent = nullptr;
            child->SetTransformDirty();
        }
    }

    void Actor::Update(const float deltaTime) {
        mIsUpdatingComponents = true;
        OnUpdate(deltaTime);

        for (const auto& component : mComponents) {
            if (component->IsEnabled()) {
                component->Update(deltaTime);
            }
        }
        mIsUpdatingComponents = false;
        FlushPendingComponents();
    }

    void Actor::FlushPendingComponents() {
        for (const Components::Component* component : mPendingRemovals) Detach(component);
        mPendingRemovals.clear();

        if (mPendingComponents.empty()) return;

        std::vector<std::unique_ptr<Components::Component>> added = std::move(mPendingComponents);
        mPendingComponents.clear();
        for (auto& component : added) {
            Attach(*component);
            mComponents.emplace_back(std::move(component));
        }
        SortComponents();
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

    void Actor::GetDebugDrawCommands(Rendering::DebugDrawList &drawList) const {
        for (const auto& component : mComponents) {
            if (component->IsEnabled()) {
                component->GetDebugDrawCommand(drawList);
            }
        }
    }

    void Actor::GetParticleDrawCommands(Rendering::ParticleDrawList &drawList) const {
        for (const auto& component : mComponents) {
            if (component->IsEnabled()) {
                component->GetParticleDrawCommand(drawList);
            }
        }
    }

    void Actor::SetComponentListener(ManagerKey, ComponentListener* listener) {
        if (listener == mComponentListener) return;

        if (mComponentListener) {
            for (const auto& component : mComponents) mComponentListener->OnComponentDetached(*component);
        }
        mComponentListener = listener;
        for (const auto& component : mComponents) Attach(*component);
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
        if (!component) return;

        if (mIsUpdatingComponents) {
            mPendingComponents.emplace_back(std::move(component));
            return;
        }

        Attach(*component);
        mComponents.emplace_back(std::move(component));
        SortComponents();
    }

    void Actor::RemoveComponent(const Components::Component* component) {
        if (!component) return;

        if (std::erase_if(mPendingComponents, [component](const auto& pending) { return pending.get() == component; }) > 0) return;

        if (mIsUpdatingComponents) {
            mPendingRemovals.push_back(component);
            return;
        }
        Detach(component);
    }

    void Actor::Attach(Components::Component& component) {
        if (mComponentListener) mComponentListener->OnComponentAttached(component);
    }

    void Actor::Detach(const Components::Component* component) {
        const auto it = std::ranges::find_if(mComponents, [component](const auto& owned) { return owned.get() == component; });
        if (it == mComponents.end()) return;

        if (mComponentListener) mComponentListener->OnComponentDetached(**it);
        mComponents.erase(it);
    }

    void Actor::SortComponents() {
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
