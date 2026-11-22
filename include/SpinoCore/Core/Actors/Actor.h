// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include <vector>
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Quaternion.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::Input {
    class InputState;
}

namespace SpinoCore::Core::Rendering {
    struct DebugDrawList;
    struct ModelDrawList;
    struct ParticleDrawList;
}

namespace SpinoCore::Core::Actors {
    namespace Components {
        class Component;
    }

    class ComponentListener;
    class EngineActorsManager;

    struct ActorInput {
        Math::Vector3f anchor{Math::Vector3f::Zero()};
        Math::Vector3f position{Math::Vector3f::Zero()};
        Math::Vector3f scale{Math::Vector3f::One()};
        Math::Quaternionf rotation{Math::Quaternionf::Identity()};
    };

    class Actor {
    public:
        enum class State{
            Active,
            Paused,
            Destroy
        };

        explicit Actor(const ActorInput& input = {});
        virtual ~Actor();

        Actor(const Actor&) = delete;
        Actor& operator=(const Actor&) = delete;
        Actor(Actor&&) = delete;
        Actor& operator=(Actor&&) = delete;

        void Update(float deltaTime);
        void ProcessInput(const Input::InputState& inputState);
        void GetDrawCommands(Rendering::ModelDrawList& drawList) const;
        void GetDebugDrawCommands(Rendering::DebugDrawList& drawList) const;
        void GetParticleDrawCommands(Rendering::ParticleDrawList& drawList) const;

        class ManagerKey { friend class EngineActorsManager; ManagerKey() = default; };
        void SetComponentListener(ManagerKey, ComponentListener* listener);

        [[nodiscard]] const Math::Vector3f& GetAnchor() const { return mAnchor; }
        void SetAnchor(const Math::Vector3f& anchor) { mAnchor = anchor; SetTransformDirty(); }

        [[nodiscard]] const Math::Vector3f& GetPosition() const { return mPosition; }
        void SetPosition(const Math::Vector3f& pos) { mPosition = pos; SetTransformDirty(); }

        [[nodiscard]] const Math::Vector3f& GetScale() const { return mScale; }
        void SetScale(const Math::Vector3f& scale) { mScale = scale; SetTransformDirty(); }

        [[nodiscard]] const Math::Quaternionf& GetRotation() const { return mRotation; }
        void SetRotation(const Math::Quaternionf& rotation) { mRotation = rotation.Normalized(); SetTransformDirty(); }

        [[nodiscard]] Math::Vector3f GetForward();
        [[nodiscard]] const Math::Matrix4f& GetWorldTransform();

        void SetParent(Actor* parent);
        [[nodiscard]] Actor* GetParent() const { return mParent; }
        [[nodiscard]] const std::vector<Actor*>& GetChildren() const { return mChildren; }

        [[nodiscard]] State GetState() const { return mState; }
        void SetState(const State state) { mState = state; }

        void SetUpdateOrder(const int order) { mUpdateOrder = order;}
        [[nodiscard]] int GetUpdateOrder() const { return mUpdateOrder; }

        [[nodiscard]] const std::vector<std::unique_ptr<Components::Component>>& GetComponents() const { return mComponents; }

        template <typename T>
        T* GetComponent() const {
            for (auto& c : mComponents){
                T* t = dynamic_cast<T*>(c.get());
                if (t != nullptr){
                    return t;
                }
            }

            return nullptr;
        }

        void AddComponent(std::unique_ptr<Components::Component> component);
        void RemoveComponent(const Components::Component* component);

    protected:
        virtual void OnUpdate(float deltaTime) = 0;
        virtual void OnProcessInput(const Input::InputState& state) = 0;

    private:
        void SetTransformDirty();
        void SortComponents();
        void Attach(Components::Component& component);
        void Detach(const Components::Component* component);
        void FlushPendingComponents();
        [[nodiscard]] bool IsAncestorOf(const Actor* actor) const;

        std::vector<std::unique_ptr<Components::Component>> mComponents;
        std::vector<std::unique_ptr<Components::Component>> mPendingComponents;
        std::vector<const Components::Component*> mPendingRemovals;
        bool mIsUpdatingComponents{false};
        ComponentListener* mComponentListener{nullptr};

        State mState{State::Active};

        Math::Vector3f mAnchor{Math::Vector3f::Zero()};
        Math::Vector3f mPosition{Math::Vector3f::Zero()};
        Math::Vector3f mScale{Math::Vector3f::One()};
        Math::Quaternionf mRotation{Math::Quaternionf::Identity()};

        Actor* mParent{nullptr};
        std::vector<Actor*> mChildren;

        Math::Matrix4f mWorldTransform{Math::Matrix4f::Identity()};
        bool mIsTransformDirty{true};
        int mUpdateOrder{100};
    };
}
