// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "ModulesInitializer.h"
#include "APIModules.h"
#include "CoreModules.h"

namespace SpinoCore::Core::Engine {
    APIModules::~APIModules() = default;
    CoreModules::~CoreModules() = default;

    bool ModulesInitializer::Initialize(APIModules &api, CoreModules &core) {
        return true;
    }
}
