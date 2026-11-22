// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "EngineActorsManager.h"
#include <algorithm>
#include <iterator>
#include <utility>

namespace SpinoCore::Core::Actors {
    Actor* EngineActorsManager::AddActor(std::unique_ptr<Actor> actor) {
        if (!actor) return nullptr;

        if (mIsUpdatingActors) {
            mPendingActors.emplace_back(std::move(actor));
            return mPendingActors.back().get();
        }

        mActors.emplace_back(std::move(actor));
        SortByUpdateOrder();
        return mActors.back().get();
    }

    void EngineActorsManager::CleanUp() {
        mActors.clear();
        mPendingActors.clear();
    }

    void EngineActorsManager::UpdateActors(const float deltaTime) {
        mIsUpdatingActors = true;
        for (const auto& actor: mActors) {
            if (actor->GetState() == Actor::State::Active) {
                actor->Update(deltaTime);
            }
        }
        mIsUpdatingActors = false;

        std::erase_if(mActors, [](const auto& actor) { return actor->GetState() == Actor::State::Destroy; });

        if (!mPendingActors.empty()) {
            std::ranges::move(mPendingActors, std::back_inserter(mActors));
            mPendingActors.clear();
            SortByUpdateOrder();
        }
    }

    void EngineActorsManager::SendInputToActors(const Input::InputState& state) const {
        for (const auto& actor: mActors) {
            if (actor->GetState() == Actor::State::Active) {
                actor->ProcessInput(state);
            }
        }
    }

    void EngineActorsManager::GetDrawCommands(Rendering::ModelDrawList& drawList) const {
        for (const auto& actor: mActors) {
            if (actor->GetState() != Actor::State::Destroy) {
                actor->GetDrawCommands(drawList);
            }
        }
    }

    void EngineActorsManager::SortByUpdateOrder() {
        std::ranges::stable_sort(mActors, [](const auto& a, const auto& b) {
            return a->GetUpdateOrder() < b->GetUpdateOrder();
        });
    }
}
