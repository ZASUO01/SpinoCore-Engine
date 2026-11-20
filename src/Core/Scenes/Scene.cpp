// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/Scenes/Scene.h"

namespace SpinoCore::Core::Scenes {
    void Scene::Start(const StartContext &ctx) {
        OnStart(ctx);
    }
}
