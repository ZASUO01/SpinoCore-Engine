// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once

namespace SpinoCore::Core::Actors::Components {
    class Component;
}

namespace SpinoCore::Core::Actors {
    class ComponentListener {
    public:
        virtual ~ComponentListener() = default;

        virtual void OnComponentAttached(Components::Component& component) = 0;
        virtual void OnComponentDetached(Components::Component& component) = 0;
    };
}
