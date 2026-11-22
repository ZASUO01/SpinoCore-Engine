// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include "Logs/LogSystem.h"
#include "SpinoCore/Logs/LogLevel.h"
#include "SpinoCore/Logs/Logger.h"
#include "SpinoCore/Logs/LogSink.h"

namespace {
    using SpinoCore::Logs::Level;
    using SpinoCore::Logs::Logger;
    using SpinoCore::Logs::LogRecord;
    using SpinoCore::Logs::LogSink;
    using SpinoCore::Logs::LogSystem;

    struct CapturedLog final {
        Level level;
        std::string message;
    };

    class RecordingSink final : public LogSink {
    public:
        explicit RecordingSink(std::vector<CapturedLog>& logs) : mLogs(logs) {}

        void Write(const LogRecord& record) override {
            mLogs.push_back({record.level, std::string{record.message}});
        }

    private:
        std::vector<CapturedLog>& mLogs;
    };

    class ThrowingSink final : public LogSink {
    public:
        void Write(const LogRecord&) override { throw std::runtime_error("sink failed"); }
    };

    class LoggingSink final : public LogSink {
    public:
        explicit LoggingSink(std::vector<CapturedLog>& logs) : mLogs(logs) {}

        void Write(const LogRecord& record) override {
            mLogs.push_back({record.level, std::string{record.message}});
            Logger::Warn("logged from inside the sink");
        }

    private:
        std::vector<CapturedLog>& mLogs;
    };

    class LoggingTests : public testing::Test {
    protected:
        void TearDown() override {
            Logger::SetLevel(Level::Info);
        }
    };

    using LogSystemTests = LoggingTests;
    using LoggerTests = LoggingTests;

    TEST_F(LogSystemTests, DeliversLoggedMessagesToItsSink) {
        std::vector<CapturedLog> logs;
        const LogSystem logSystem{std::make_unique<RecordingSink>(logs)};

        Logger::Info("engine started");

        ASSERT_EQ(logs.size(), 1u);
        EXPECT_EQ(logs[0].message, "engine started");
    }

    TEST_F(LogSystemTests, CreatingASecondLogSystemWhileOneIsAliveThrows) {
        std::vector<CapturedLog> logs;
        const LogSystem firstLogSystem{std::make_unique<RecordingSink>(logs)};

        EXPECT_THROW(LogSystem{std::make_unique<RecordingSink>(logs)}, std::logic_error);
    }

    TEST_F(LogSystemTests, CanBeCreatedAgainAfterThePreviousOneIsDestroyed) {
        std::vector<CapturedLog> logs;
        { const LogSystem firstLogSystem{std::make_unique<RecordingSink>(logs)}; }

        EXPECT_NO_THROW(LogSystem{std::make_unique<RecordingSink>(logs)});
    }

    TEST_F(LogSystemTests, MessagesLoggedAfterTheLogSystemIsDestroyedAreDropped) {
        std::vector<CapturedLog> logs;
        { const LogSystem logSystem{std::make_unique<RecordingSink>(logs)}; }

        Logger::Info("nobody listening");

        EXPECT_TRUE(logs.empty());
    }

    TEST_F(LoggerTests, FormatsTheMessageWithItsArguments) {
        std::vector<CapturedLog> logs;
        const LogSystem logSystem{std::make_unique<RecordingSink>(logs)};

        Logger::Info("player {} joined with {} hp", 7, 100);

        ASSERT_EQ(logs.size(), 1u);
        EXPECT_EQ(logs[0].message, "player 7 joined with 100 hp");
    }

    TEST_F(LoggerTests, TagsEachMessageWithTheLevelItWasLoggedAt) {
        std::vector<CapturedLog> logs;
        const LogSystem logSystem{std::make_unique<RecordingSink>(logs)};

        Logger::Error("disk full");

        ASSERT_EQ(logs.size(), 1u);
        EXPECT_EQ(logs[0].level, Level::Error);
    }

    TEST_F(LoggerTests, DropsMessagesBelowTheConfiguredLevel) {
        std::vector<CapturedLog> logs;
        const LogSystem logSystem{std::make_unique<RecordingSink>(logs)};
        Logger::SetLevel(Level::Error);

        Logger::Warn("low fuel");

        EXPECT_TRUE(logs.empty());
    }

    TEST_F(LoggerTests, DeliversMessagesAtTheConfiguredLevel) {
        std::vector<CapturedLog> logs;
        const LogSystem logSystem{std::make_unique<RecordingSink>(logs)};
        Logger::SetLevel(Level::Warn);

        Logger::Warn("low fuel");

        EXPECT_EQ(logs.size(), 1u);
    }

    TEST_F(LoggerTests, ForcedMessagesAreDeliveredEvenWhenLoggingIsDisabled) {
        std::vector<CapturedLog> logs;
        const LogSystem logSystem{std::make_unique<RecordingSink>(logs)};
        Logger::SetLevel(Level::None);

        Logger::Force("core message");

        EXPECT_EQ(logs.size(), 1u);
    }

    TEST_F(LogSystemTests, AFailingSinkDoesNotStopTheLogger) {
        const LogSystem logSystem{std::make_unique<ThrowingSink>()};

        EXPECT_NO_THROW(Logger::Error("this sink will throw"));
    }

    TEST_F(LogSystemTests, MessagesLoggedFromInsideASinkAreDropped) {
        std::vector<CapturedLog> logs;
        const LogSystem logSystem{std::make_unique<LoggingSink>(logs)};

        Logger::Info("outer message");

        ASSERT_EQ(logs.size(), 1u);
        EXPECT_EQ(logs[0].message, "outer message");
    }
}
