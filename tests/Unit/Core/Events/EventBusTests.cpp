// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <optional>
#include <utility>
#include <vector>
#include <gtest/gtest.h>
#include "Core/Events/EventBus.h"
#include "Core/Events/TypeEvents/TypeEvent.h"

namespace {
    using SpinoCore::Core::Events::EventBus;
    using SpinoCore::Core::Events::TypeEvent;

    struct PingEvent final : TypeEvent {
        int value{0};
    };

    struct OtherEvent final : TypeEvent {};
    struct LateEvent final : TypeEvent {};

    struct Recorder final {
        std::vector<int> calls{};
        EventBus* bus{nullptr};
        std::optional<EventBus::Subscription> extra{};
    };

    void RecordOne(PingEvent& /*event*/, void* context) { static_cast<Recorder*>(context)->calls.push_back(1); }
    void RecordTwo(PingEvent& /*event*/, void* context) { static_cast<Recorder*>(context)->calls.push_back(2); }
    void RecordValue(PingEvent& event, void* context) { static_cast<Recorder*>(context)->calls.push_back(event.value); } // NOLINT
    void RecordOther(OtherEvent& /*event*/, void* context) { static_cast<Recorder*>(context)->calls.push_back(99); }

    void StopAfterRecording(PingEvent& event, void* context) {
        RecordOne(event, context);
        event.stopPropagation = true;
    }

    void UnsubscribeSelf(PingEvent& event, void* context) {
        RecordOne(event, context);
        static_cast<Recorder*>(context)->extra.reset();
    }

    void SubscribeToANewEventType(PingEvent& event, void* context) {
        RecordOne(event, context);
        auto* recorder = static_cast<Recorder*>(context);
        recorder->extra = recorder->bus->Listen<LateEvent>([](LateEvent&, void*) {});
    }

    class EventBusTests : public testing::Test {
    protected:
        EventBus bus{};
        Recorder recorder{};
    };

    TEST_F(EventBusTests, DeliversADispatchedEventToItsListener) {
        auto subscription = bus.Listen<PingEvent>(&RecordValue, &recorder);
        PingEvent event{};
        event.value = 7;

        bus.Dispatch(event);

        EXPECT_EQ(recorder.calls, std::vector{7});
    }

    TEST_F(EventBusTests, DeliversToListenersInTheOrderTheySubscribed) {
        auto first = bus.Listen<PingEvent>(&RecordOne, &recorder);
        auto second = bus.Listen<PingEvent>(&RecordTwo, &recorder);
        PingEvent event{};

        bus.Dispatch(event);

        EXPECT_EQ(recorder.calls, (std::vector{1, 2}));
    }

    TEST_F(EventBusTests, DoesNotDeliverOtherEventTypes) {
        auto subscription = bus.Listen<OtherEvent>(&RecordOther, &recorder);
        PingEvent event{};

        bus.Dispatch(event);

        EXPECT_TRUE(recorder.calls.empty());
    }

    TEST_F(EventBusTests, DestroyingTheSubscriptionStopsDelivery) {
        {
            auto subscription = bus.Listen<PingEvent>(&RecordOne, &recorder);
        }
        PingEvent event{};

        bus.Dispatch(event);

        EXPECT_TRUE(recorder.calls.empty());
    }

    TEST_F(EventBusTests, AMovedSubscriptionKeepsListening) {
        auto original = bus.Listen<PingEvent>(&RecordOne, &recorder);
        EventBus::Subscription moved{std::move(original)};
        original.Reset();
        PingEvent event{};

        bus.Dispatch(event);

        EXPECT_EQ(recorder.calls, std::vector{1});
    }

    TEST_F(EventBusTests, StopPropagationSkipsTheRemainingListeners) {
        auto first = bus.Listen<PingEvent>(&StopAfterRecording, &recorder);
        auto second = bus.Listen<PingEvent>(&RecordTwo, &recorder);
        PingEvent event{};

        bus.Dispatch(event);

        EXPECT_EQ(recorder.calls, std::vector{1});
    }

    TEST_F(EventBusTests, AListenerCanUnsubscribeItselfDuringDispatch) {
        recorder.extra = bus.Listen<PingEvent>(&UnsubscribeSelf, &recorder);
        auto other = bus.Listen<PingEvent>(&RecordTwo, &recorder);
        PingEvent first{};
        PingEvent second{};

        bus.Dispatch(first);
        bus.Dispatch(second);

        EXPECT_EQ(recorder.calls, (std::vector{1, 2, 2}));
    }

    TEST_F(EventBusTests, AListenerCanSubscribeToANewEventTypeDuringDispatch) {
        recorder.bus = &bus;
        auto subscriber = bus.Listen<PingEvent>(&SubscribeToANewEventType, &recorder);
        auto other = bus.Listen<PingEvent>(&RecordTwo, &recorder);
        PingEvent event{};

        bus.Dispatch(event);

        EXPECT_EQ(recorder.calls, (std::vector{1, 2}));
    }

    TEST_F(EventBusTests, MemberFunctionsCanListen) {
        struct Listener final {
            int received{0};
            void OnPing(const PingEvent& event) { received = event.value; }
        } listener;
        auto subscription = bus.Listen<PingEvent, &Listener::OnPing>(&listener);
        PingEvent event{};
        event.value = 3;

        bus.Dispatch(event);

        EXPECT_EQ(listener.received, 3);
    }
}
