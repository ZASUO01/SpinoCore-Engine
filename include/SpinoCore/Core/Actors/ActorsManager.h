// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>

namespace SpinoCore::Core::Actors {
    class Actor;

    class ActorsManager {
    public:
        virtual ~ActorsManager() = default;

        virtual Actor* AddActor(std::unique_ptr<Actor> actor) = 0;
        virtual void CleanUp() = 0;
    };
}
