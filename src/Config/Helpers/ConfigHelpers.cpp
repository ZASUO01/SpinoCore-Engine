// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "ConfigHelpers.h"
#include "SpinoCore/Config/Store.h"

namespace SpinoCore::Config::Helpers {
    void SetInitialConfig(const RootConfig& rootConfig, Store& store) {
        // logs
        store.Set("engine.loglevel", rootConfig.logLevel, true);

        // app
        store.Set("engine.app.name", rootConfig.app.name, true);
        store.Set("engine.app.version", rootConfig.app.version, true);
        store.Set("engine.app.identifier", rootConfig.app.identifier, true);
        store.Set("engine.app.creator", rootConfig.app.creator, true);
        store.Set("engine.app.url", rootConfig.app.url, true);

        // window
        store.Set("engine.window.title", rootConfig.window.title, true);
        store.Set("engine.window.width", rootConfig.window.initialWidth, true);
        store.Set("engine.window.height", rootConfig.window.initialHeight, true);
    }
}