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

/// @brief Actors: the objects that live in a scene, and the components that give them behaviour.
namespace SpinoCore::Core::Actors {
    namespace Components {
        class Component;
    }

    class ComponentListener;
    class EngineActorsManager;

    /// @brief Initial transform of an Actor.
    struct ActorInput {
        /// Offset added to the position when the transform is built; it moves the actor without changing its rotation pivot.
        Math::Vector3f anchor{Math::Vector3f::Zero()};
        /// Position relative to the parent, or to the world when there is no parent.
        Math::Vector3f position{Math::Vector3f::Zero()};
        /// Scale on each axis.
        Math::Vector3f scale{Math::Vector3f::One()};
        /// Rotation; it is normalized by the constructor.
        Math::Quaternionf rotation{Math::Quaternionf::Identity()};
    };

    /// @brief Base class of every object placed in a scene.
    ///
    /// An actor has a transform, an optional parent, a list of components and a state.
    /// Subclasses implement OnUpdate and OnProcessInput. Actors are owned by the ActorsManager.
    class Actor {
    public:
        /// @brief Lifecycle state of an actor.
        enum class State{
            /// Receives input and updates.
            Active,
            /// Is drawn but does not update or receive input.
            Paused,
            /// Is removed by the manager after the current update.
            Destroy
        };

        /// @brief Creates an actor with the given transform.
        /// @param input Initial anchor, position, scale and rotation.
        explicit Actor(const ActorInput& input = {});
        /// @brief Detaches the components and unlinks the parent and the children.
        virtual ~Actor();

        Actor(const Actor&) = delete;
        Actor& operator=(const Actor&) = delete;
        Actor(Actor&&) = delete;
        Actor& operator=(Actor&&) = delete;

        /// @brief Runs OnUpdate and then updates the enabled components.
        /// @param deltaTime Fixed simulation step in seconds.
        void Update(float deltaTime);
        /// @brief Forwards the simulation input to OnProcessInput.
        /// @param inputState Input of the current simulation step.
        void ProcessInput(const Input::InputState& inputState);
        /// @brief Collects the model draw commands of the enabled components.
        /// @param drawList List that receives the commands.
        void GetDrawCommands(Rendering::ModelDrawList& drawList) const;
        /// @brief Collects the debug shapes of the enabled components.
        /// @param drawList List that receives the shapes.
        void GetDebugDrawCommands(Rendering::DebugDrawList& drawList) const;
        /// @brief Collects the particles of the enabled components.
        /// @param drawList List that receives the particles.
        void GetParticleDrawCommands(Rendering::ParticleDrawList& drawList) const;

        /// @brief Passkey that only the engine actors manager can build.
        class ManagerKey { friend class EngineActorsManager; ManagerKey() = default; };
        /// @brief Sets the listener notified when components are attached or detached.
        /// @param listener Listener owned by the engine, or nullptr.
        void SetComponentListener(ManagerKey, ComponentListener* listener);

        /// @return The anchor offset.
        [[nodiscard]] const Math::Vector3f& GetAnchor() const { return mAnchor; }
        /// @brief Changes the anchor offset.
        /// @param anchor New offset.
        void SetAnchor(const Math::Vector3f& anchor) { mAnchor = anchor; SetTransformDirty(); }

        /// @return The position relative to the parent.
        [[nodiscard]] const Math::Vector3f& GetPosition() const { return mPosition; }
        /// @brief Moves the actor.
        /// @param pos New position relative to the parent.
        void SetPosition(const Math::Vector3f& pos) { mPosition = pos; SetTransformDirty(); }

        /// @return The scale on each axis.
        [[nodiscard]] const Math::Vector3f& GetScale() const { return mScale; }
        /// @brief Changes the scale.
        /// @param scale New scale on each axis.
        void SetScale(const Math::Vector3f& scale) { mScale = scale; SetTransformDirty(); }

        /// @return The normalized rotation.
        [[nodiscard]] const Math::Quaternionf& GetRotation() const { return mRotation; }
        /// @brief Changes the rotation.
        /// @param rotation New rotation; it is normalized before being stored.
        void SetRotation(const Math::Quaternionf& rotation) { mRotation = rotation.Normalized(); SetTransformDirty(); }

        /// @return The unit vector the actor faces in world space (its local -Z axis).
        [[nodiscard]] Math::Vector3f GetForward();
        /// @return The world transform, rebuilt only when the actor or one of its parents changed.
        [[nodiscard]] const Math::Matrix4f& GetWorldTransform();

        /// @brief Attaches the actor to a parent, keeping the local transform.
        ///
        /// A parent that would create a cycle is ignored.
        /// @param parent New parent, or nullptr to detach.
        void SetParent(Actor* parent);
        /// @return The parent, or nullptr.
        [[nodiscard]] Actor* GetParent() const { return mParent; }
        /// @return The actors attached to this one.
        [[nodiscard]] const std::vector<Actor*>& GetChildren() const { return mChildren; }

        /// @return The lifecycle state.
        [[nodiscard]] State GetState() const { return mState; }
        /// @brief Changes the lifecycle state.
        /// @param state New state; State::Destroy removes the actor after the update.
        void SetState(const State state) { mState = state; }

        /// @brief Changes the update order; lower values update first.
        /// @param order New order.
        void SetUpdateOrder(const int order) { mUpdateOrder = order;}
        /// @return The update order.
        [[nodiscard]] int GetUpdateOrder() const { return mUpdateOrder; }

        /// @return The attached components, sorted by update order.
        [[nodiscard]] const std::vector<std::unique_ptr<Components::Component>>& GetComponents() const { return mComponents; }

        /// @brief Finds the first component of a given type.
        /// @tparam T Component type.
        /// @return The component, or nullptr when there is none.
        template <typename T>
        T* GetComponent() const {
            for (const auto& c : mComponents){
                T* t = dynamic_cast<T*>(c.get());
                if (t != nullptr){
                    return t;
                }
            }

            return nullptr;
        }

        /// @brief Attaches a component; during an update it is attached at the end of the update.
        /// @param component Component to own; nullptr is ignored.
        void AddComponent(std::unique_ptr<Components::Component> component);
        /// @brief Detaches and destroys a component; during an update it is removed at the end of the update.
        /// @param component Component to remove.
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
