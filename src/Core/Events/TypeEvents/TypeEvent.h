// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <atomic>
#include <cstdint>

namespace SpinoCore::Core::Events {
    struct TypeEvent {
        bool stopPropagation{false};

        template <typename TEvent>
        [[nodiscard]] static std::uint32_t ID() noexcept {
            static const std::uint32_t id = sNextID.fetch_add(1, std::memory_order_relaxed);
            return id;
        }

    private:
        static inline std::atomic<std::uint32_t> sNextID{0};
    };
}