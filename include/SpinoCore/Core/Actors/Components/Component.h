// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once

namespace SpinoCore::Core::Actors {
    class Actor;
}

namespace SpinoCore::Core::Rendering {
    struct ModelDrawList;
    struct DebugDrawList;
    struct ParticleDrawList;
}

/// @brief Components that add behaviour, rendering and physics to actors.
namespace SpinoCore::Core::Actors::Components {
    /// @brief Initial settings shared by every component.
    struct ComponentInput {
        /// Update order inside the actor; lower values update first.
        int mUpdateOrder{100};
        /// Draw order used by the renderer.
        int mDrawOrder{100};
        /// Whether the component starts enabled.
        bool mIsEnabled{true};
    };

    /// @brief Base class of every actor component.
    class Component {
    public:
        /// @brief Creates a component for an actor.
        /// @param actor Owner of the component.
        /// @param input Initial settings.
        explicit Component(Actor &actor, const ComponentInput& input = {});
        virtual ~Component() = default;

        /// @brief Called once per simulation step while the component is enabled.
        virtual void Update(float) {}
        /// @brief Adds the model draw commands of the component.
        virtual void GetDrawCommand(Rendering::ModelDrawList&) {}
        /// @brief Adds the debug shapes of the component.
        virtual void GetDebugDrawCommand(Rendering::DebugDrawList&) {}
        /// @brief Adds the particles of the component.
        virtual void GetParticleDrawCommand(Rendering::ParticleDrawList&) {}

        /// @return The update order.
        [[nodiscard]] int GetUpdateOrder() const { return mUpdateOrder; }
        /// @brief Changes the update order.
        /// @param updateOrder New order; lower values update first.
        void SetUpdateOrder(const int updateOrder) { mUpdateOrder = updateOrder; }

        /// @return The draw order.
        [[nodiscard]] int GetDrawOrder() const { return mDrawOrder; }
        /// @brief Changes the draw order.
        /// @param drawOrder New order.
        void SetDrawOrder(const int drawOrder) { mDrawOrder = drawOrder; }

        /// @brief Enables or disables the component.
        /// @param enabled true to update and draw it.
        void SetEnabled(const bool enabled) { mIsEnabled = enabled; }
        /// @return Whether the component updates and draws.
        [[nodiscard]] bool IsEnabled() const { return mIsEnabled; }

        /// @return The owner of the component.
        [[nodiscard]] Actor* GetActor() const { return &mOwner; }

    private:
        Actor& mOwner;
        int mUpdateOrder{100};
        int mDrawOrder{100};
        bool mIsEnabled{true};
    };
}