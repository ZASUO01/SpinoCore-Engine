// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace SpinoCore::Utils {
    template <typename T>
    class ThreadSafeQueue {
    public:
        ThreadSafeQueue() = default;
        ~ThreadSafeQueue() = default;

        ThreadSafeQueue(const ThreadSafeQueue&) = delete;
        ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
        ThreadSafeQueue(ThreadSafeQueue&&) = delete;
        ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;


        void Push(T item) {
            std::lock_guard lock(mMutex);
            mQueue.push(std::move(item));
            mConditionVariable.notify_one();
        }

        [[nodiscard]] std::optional<T> TryPop() {
            std::lock_guard lock(mMutex);
            if (mQueue.empty()) {
                return std::nullopt;
            }

            auto item = std::move(mQueue.front());
            mQueue.pop();
            return item;
        }

        [[nodiscard]] std::optional<T> WaitAndPop(std::stop_token stopToken) {
            std::unique_lock lock(mMutex);

            const bool signaled = mConditionVariable.wait(lock, stopToken, [this] {
                return !mQueue.empty();
            });

            if (!signaled || stopToken.stop_requested()) {
                return std::nullopt;
            }

            auto item = std::move(mQueue.front());
            mQueue.pop();
            return item;
        }

        [[nodiscard]] bool IsEmpty() const {
            std::lock_guard lock(mMutex);
            return mQueue.empty();
        }

        void Clear() {
            std::lock_guard lock(mMutex);
            std::queue<T> emptyQueue;
            std::swap(mQueue, emptyQueue);
        }

    private:
        mutable std::mutex mMutex;
        std::queue<T> mQueue;
        std::condition_variable_any mConditionVariable;
    };
}
