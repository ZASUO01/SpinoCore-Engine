// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <concepts>
#include <cstddef>
#include "TypeEvents/TypeEvent.h"
#include <cstdint>
#include <utility>
#include <vector>

namespace SpinoCore::Core::Events {
    class EventBus final {
    public:
        template <std::derived_from<TypeEvent> TEvent>
        using Callback = void (*)(TEvent& event, void* context);

        class Subscription final {
        public:
            Subscription() = default;
            ~Subscription() { Reset(); }

            Subscription(const Subscription&) = delete;
            Subscription& operator=(const Subscription&) = delete;

            Subscription(Subscription&& other) noexcept
                : mBus(std::exchange(other.mBus, nullptr)), mEventID(other.mEventID), mListenerID(other.mListenerID) {}

            Subscription& operator=(Subscription&& other) noexcept {
                if (this != &other) {
                    Reset();
                    mBus = std::exchange(other.mBus, nullptr);
                    mEventID = other.mEventID;
                    mListenerID = other.mListenerID;
                }
                return *this;
            }

            void Reset() noexcept {
                if (mBus) mBus->Unlisten(mEventID, mListenerID);
                mBus = nullptr;
            }

        private:
            friend class EventBus;

            Subscription(EventBus* bus, const std::uint32_t eventID, const std::uint64_t listenerID) noexcept
                : mBus(bus), mEventID(eventID), mListenerID(listenerID) {}

            EventBus* mBus{nullptr};
            std::uint32_t mEventID{0};
            std::uint64_t mListenerID{0};
        };

        EventBus() = default;
        ~EventBus() = default;

        EventBus(const EventBus&) = delete;
        EventBus& operator=(const EventBus&) = delete;
        EventBus(EventBus&&) = delete;
        EventBus& operator=(EventBus&&) = delete;

        template <std::derived_from<TypeEvent> TEvent>
        [[nodiscard]] Subscription Listen(const Callback<TEvent> callback, void* context = nullptr) {
            const auto uniqueEventID = TypeEvent::ID<TEvent>();
            if (uniqueEventID >= mListeners.size()) {
                mListeners.resize(uniqueEventID + 1);
            }

            const std::uint64_t listenerID = ++mLastListenerID;
            mListeners[uniqueEventID].push_back({
                .callback = reinterpret_cast<EmptyCallback>(callback),
                .context = context,
                .id = listenerID,
            });

            return Subscription{this, uniqueEventID, listenerID};
        }

        // Shortcut for listening with a member function: bus.Listen<MyEvent, &MyClass::OnMyEvent>(this)
        template <std::derived_from<TypeEvent> TEvent, auto Method, typename TOwner>
        [[nodiscard]] Subscription Listen(TOwner* owner) {
            return Listen<TEvent>([](TEvent& event, void* context) {
                (static_cast<TOwner*>(context)->*Method)(event);
            }, owner);
        }

        template <std::derived_from<TypeEvent> TEvent>
        void Dispatch(TEvent& event) {
            const auto uniqueEventID = TypeEvent::ID<TEvent>();
            if (uniqueEventID >= mListeners.size()) return;

            const std::vector<Listener>& listeners = mListeners[uniqueEventID];
            const std::size_t count = listeners.size();

            ++mDispatchDepth;
            for (std::size_t index = 0; index < count && !event.stopPropagation; ++index) {
                const auto [callback, context, id] = listeners[index];
                if (!callback) continue;
                reinterpret_cast<Callback<TEvent>>(callback)(event, context);
            }
            --mDispatchDepth;

            if (mDispatchDepth == 0 && mHasRemovedListeners) RemoveInactiveListeners();
        }

    private:
        using EmptyCallback = void (*)();

        struct Listener final {
            EmptyCallback callback;
            void* context;
            std::uint64_t id;
        };
        void Unlisten(const std::uint32_t eventID, const std::uint64_t listenerID) noexcept {
            std::vector<Listener>& listeners = mListeners[eventID];
            for (std::size_t index = 0; index < listeners.size(); ++index) {
                if (listeners[index].id != listenerID) continue;

                if (mDispatchDepth > 0) {
                    listeners[index].callback = nullptr;
                    mHasRemovedListeners = true;
                } else {
                    listeners.erase(listeners.begin() + static_cast<std::ptrdiff_t>(index));
                }
                return;
            }
        }

        void RemoveInactiveListeners() noexcept {
            for (std::vector<Listener>& listeners : mListeners) {
                std::erase_if(listeners, [](const Listener& listener) { return !listener.callback; });
            }
            mHasRemovedListeners = false;
        }

        std::vector<std::vector<Listener>> mListeners;
        std::uint64_t mLastListenerID{0};
        std::uint32_t mDispatchDepth{0};
        bool mHasRemovedListeners{false};
    };
}