// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <optional>
#include "Structs.h"

namespace SpinoCore::SDL {
    class Context {
    public:
        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;

        Context(Context&& other) noexcept;
        Context& operator=(Context&& other) noexcept;

        ~Context();

        static std::optional<Context> Create(const AppMetadata& metadata);

    private:
        Context();
        bool mOwnsSDL = false;
        static bool Initialize(const AppMetadata& metadata);
    };
}