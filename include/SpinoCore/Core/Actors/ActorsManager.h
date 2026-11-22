// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>

namespace SpinoCore::Core::Actors {
    class Actor;

    /// @brief Owns the actors of the current scene.
    class ActorsManager {
    public:
        virtual ~ActorsManager() = default;

        /// @brief Adds an actor; actors added during an update start on the next update.
        /// @param actor Actor to own; nullptr is ignored.
        /// @return The added actor, or nullptr.
        virtual Actor* AddActor(std::unique_ptr<Actor> actor) = 0;
        /// @brief Destroys every actor.
        virtual void CleanUp() = 0;
    };
}
