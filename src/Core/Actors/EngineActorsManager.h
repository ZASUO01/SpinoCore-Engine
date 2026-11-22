// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include <vector>
#include "SpinoCore/Core/Actors/Actor.h"
#include "SpinoCore/Core/Actors/ActorsManager.h"
#include "Core/Actors/ComponentListener.h"

namespace SpinoCore::Physics {
    class CollisionProcessor;
}

namespace SpinoCore::Core::Input {
    class InputState;
}

namespace SpinoCore::Core::Rendering {
    struct DebugDrawList;
    struct ModelDrawList;
    struct ParticleDrawList;
}

namespace SpinoCore::Core::Actors {
    class EngineActorsManager final : public ActorsManager, private ComponentListener {
    public:
        explicit EngineActorsManager(Physics::CollisionProcessor* collisionProcessor = nullptr);
        ~EngineActorsManager() override;

        EngineActorsManager(const EngineActorsManager&) = delete;
        EngineActorsManager& operator=(const EngineActorsManager&) = delete;
        EngineActorsManager(EngineActorsManager&&) = delete;
        EngineActorsManager& operator=(EngineActorsManager&&) = delete;

        Actor* AddActor(std::unique_ptr<Actor> actor) override;
        void CleanUp() override;

        void UpdateActors(float deltaTime);
        void SendInputToActors(const Input::InputState& state) const;
        void GetDrawCommands(Rendering::ModelDrawList& drawList) const;
        void GetDebugDrawCommands(Rendering::DebugDrawList& drawList) const;
        void GetParticleDrawCommands(Rendering::ParticleDrawList& drawList) const;

        [[nodiscard]] const std::vector<std::unique_ptr<Actor>>& GetActors() const { return mActors; }

    private:
        void SortByUpdateOrder();
        void OnComponentAttached(Components::Component& component) override;
        void OnComponentDetached(Components::Component& component) override;

        Physics::CollisionProcessor* mCollisionProcessor{nullptr};
        std::vector<std::unique_ptr<Actor>> mActors;
        std::vector<std::unique_ptr<Actor>> mPendingActors;
        bool mIsUpdatingActors{false};
    };
}
