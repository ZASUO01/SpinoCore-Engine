// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <array>
#include <atomic>
#include <cstddef>

namespace SpinoCore::Sound::Synth {
    template <typename T, std::size_t Capacity>
    class CommandQueue final {
    public:
        bool Push(const T& item) noexcept {
            const std::size_t tail = mTail.load(std::memory_order_relaxed);
            const std::size_t next = (tail + 1) % Capacity;
            if (next == mHead.load(std::memory_order_acquire)) return false;

            mItems[tail] = item;
            mTail.store(next, std::memory_order_release);
            return true;
        }

        bool Pop(T& item) noexcept {
            const std::size_t head = mHead.load(std::memory_order_relaxed);
            if (head == mTail.load(std::memory_order_acquire)) return false;

            item = mItems[head];
            mHead.store((head + 1) % Capacity, std::memory_order_release);
            return true;
        }

        [[nodiscard]] bool IsEmpty() const noexcept {
            return mHead.load(std::memory_order_acquire) == mTail.load(std::memory_order_acquire);
        }

    private:
        std::array<T, Capacity> mItems{};
        std::atomic<std::size_t> mHead{0};
        std::atomic<std::size_t> mTail{0};
    };
}
