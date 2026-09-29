// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <gtest/gtest.h>
#include "Logs/LogSink.h"
#include "Logs/LogSystem.h"
#include "SpinoCore/Logs/Logger.h"

using namespace SpinoCore::Logs;

namespace {
    struct LogRecord {
        Level level;
        std::string message;
    };

    class MemoryMockSink final : public LogSink {
    public:
        explicit MemoryMockSink(std::vector<LogRecord>& records) :mRecords(records) {}

        void Write(const Level level, const zoned_time<system_clock::duration> & /*time*/, const std::string_view message) override {
            mRecords.push_back({.level = level, .message = std::string(message)});
        }

    private:
        std::vector<LogRecord>& mRecords;
    };

    class LogsUnitTest : public testing::Test {
    protected:
        std::vector<LogRecord> mLogOutput;

        void SetUp() override {
            LogSystem::Shutdown();
            mLogOutput.clear();

            LogSystem::Initialize();
            LogSystem::SetSink(std::make_unique<MemoryMockSink>(mLogOutput));
            Logger::SetLevel(Level::Info);
        }

        void TearDown() override {
            LogSystem::Shutdown();
        }
    };
}

TEST_F(LogsUnitTest, StringFormattingShouldWork) {
    Logger::Info("The number {} is a {} number.", 42.5f, "float");

    ASSERT_EQ(mLogOutput.size(), 1);
    EXPECT_EQ(mLogOutput[0].level, Level::Info);
    EXPECT_EQ(mLogOutput[0].message, "The number 42.5 is a float number.");
}

TEST_F(LogsUnitTest, LogSeverityIsFiltered) {
    Logger::SetLevel(Level::Error);

    Logger::Info("Info ignored");
    Logger::Warn("Warn ignored");
    Logger::Error("Error not ignored");
    Logger::Fatal("Fatal not ignored");

    ASSERT_EQ(mLogOutput.size(), 2);
    ASSERT_EQ(mLogOutput[0].level, Level::Error);
    ASSERT_EQ(mLogOutput[1].level, Level::Fatal);
}

TEST_F(LogsUnitTest, ForceByPassesFiltering) {
    Logger::SetLevel(Level::None);

    Logger::Fatal("Even fatal is ignored");
    Logger::Force("Force is not ignored");

    ASSERT_EQ(mLogOutput.size(), 1);
    EXPECT_EQ(mLogOutput[0].level, Level::Force);
}

TEST_F(LogsUnitTest, SystemIsProtectedAgainstNullSink) {
    LogSystem::SetSink(nullptr);

    EXPECT_NO_FATAL_FAILURE({
        Logger::Info("Info to empty sink");
        Logger::Force("Force to empty sink");
    });
}

TEST_F(LogsUnitTest, SystemIsThreadSafe) {
    constexpr int NUM_THREADS = 10;
    constexpr int LOGS_PER_THREAD = 1000;
    std::vector<std::thread> workers;

    for (int i = 0; i < NUM_THREADS; ++i) {
        workers.emplace_back([i] {
            for (int j = 0; j < LOGS_PER_THREAD; ++j) {
                Logger::Info("Thead {} logs {}", i, j);
            }
        });
    }

    for (auto& t: workers) {
        t.join();
    }

    EXPECT_EQ(mLogOutput.size(), NUM_THREADS * LOGS_PER_THREAD);
}