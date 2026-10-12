// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "FrameOperations.h"
#include "Defs/FrameOperationsConcept.h"
#include "Core/Events/Handler.h"

namespace SpinoCore::Runtime {
    static_assert(Defs::IsFrameOperator<FrameOperations>);

    FrameOperations::FrameOperations(Core::Engine::CoreModules& core) noexcept : mCore(core) {}

    bool FrameOperations::ProcessEvents() {
        return Core::Events::Handler::PollEvents();
    }

    void FrameOperations::UpdateSimulation(float /*deltaTime*/) {}
    void FrameOperations::InterpolateEntities(float /*alpha*/) {}
    void FrameOperations::Render() {}
}