// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <gtest/gtest.h>
#include "Utils/ThreadSafeQueue.h"
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>

using namespace SpinoCore::Utils;

class ThreadSafeQueueTest : public testing::Test {
protected:
    ThreadSafeQueue<int> intQueue;
};


TEST_F(ThreadSafeQueueTest, IsEmptyOnCreation) {
    EXPECT_TRUE(intQueue.IsEmpty());
    EXPECT_FALSE(intQueue.TryPop().has_value());
}

TEST_F(ThreadSafeQueueTest, PushAndTryPop) {
    intQueue.Push(42);
    EXPECT_FALSE(intQueue.IsEmpty());

    const auto result = intQueue.TryPop();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 42);
    EXPECT_TRUE(intQueue.IsEmpty());
}

TEST_F(ThreadSafeQueueTest, ClearRemovesAllElements) {
    intQueue.Push(1);
    intQueue.Push(2);
    intQueue.Push(3);

    intQueue.Clear();

    EXPECT_TRUE(intQueue.IsEmpty());
    EXPECT_FALSE(intQueue.TryPop().has_value());
}


TEST_F(ThreadSafeQueueTest, WaitAndPopReturnsDataWhenPushed) {
    std::stop_source stopSource;
    std::atomic workerFinished{false};
    int poppedValue = 0;

    std::jthread worker([&](const std::stop_token &st) {
        if (const auto result = intQueue.WaitAndPop(st)) {
            poppedValue = *result;
        }
        workerFinished = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    intQueue.Push(99);

    worker.join();
    EXPECT_TRUE(workerFinished);
    EXPECT_EQ(poppedValue, 99);
}

TEST_F(ThreadSafeQueueTest, WaitAndPopRespectsStopToken) {
    std::stop_source stopSource;
    std::atomic workerFinished{false};
    bool hasValue = true;

    std::jthread worker([&](const std::stop_token &st) {
        const auto result = intQueue.WaitAndPop(st);
        hasValue = result.has_value();
        workerFinished = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    worker.request_stop();
    worker.join();

    EXPECT_TRUE(workerFinished);
    EXPECT_FALSE(hasValue);
}

TEST_F(ThreadSafeQueueTest, MultiProducerSingleConsumer) {
    constexpr int NUM_PRODUCERS = 10;
    constexpr int ITEMS_PER_PRODUCER = 1000;
    constexpr int EXPECTED_TOTAL = NUM_PRODUCERS * ITEMS_PER_PRODUCER;

    std::vector<std::jthread> producers;

    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        producers.emplace_back([this, i] {
            for (int j = 0; j < ITEMS_PER_PRODUCER; ++j) {
                intQueue.Push(i * ITEMS_PER_PRODUCER + j);
            }
        });
    }

    producers.clear();

    int totalPops = 0;
    while (intQueue.TryPop()) {
        totalPops++;
    }

    EXPECT_EQ(totalPops, EXPECTED_TOTAL);
    EXPECT_TRUE(intQueue.IsEmpty());
}

TEST_F(ThreadSafeQueueTest, SingleProducerMultiConsumer) {
    constexpr int TOTAL_ITEMS = 10000;
    constexpr int NUM_CONSUMERS = 10;
    std::atomic consumedCount{0};

    for (int i = 0; i < TOTAL_ITEMS; ++i) {
        intQueue.Push(i);
    }

    std::vector<std::jthread> consumers;

    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        consumers.emplace_back([this, &consumedCount] {
            while (intQueue.TryPop()) {
                ++consumedCount;
            }
        });
    }

    consumers.clear();

    EXPECT_EQ(consumedCount.load(), TOTAL_ITEMS);
    EXPECT_TRUE(intQueue.IsEmpty());
}