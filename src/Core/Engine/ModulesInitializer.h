// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once

namespace SpinoCore::Core::Engine {
    struct APIModules;
    struct CoreModules;

    class ModulesInitializer final {
    public:
        ModulesInitializer() = delete;

        [[nodiscard]] static bool Initialize(APIModules& api, CoreModules& core);

    private:
        [[nodiscard]] static bool InitializeFileModules(APIModules& api, CoreModules& core);
        static void InitializeConfigModule(const APIModules& api, CoreModules& core);
    };
}