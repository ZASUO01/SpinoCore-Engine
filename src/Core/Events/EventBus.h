// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>
#include "TypeEvents/TypeEvent.h"

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
        ~EventBus() {
            assert(HasNoListeners() && "EventBus destroyed while a Subscription is still alive");
        }

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

            const std::size_t count = mListeners[uniqueEventID].size();

            ++mDispatchDepth;
            for (std::size_t index = 0; index < count && !event.stopPropagation; ++index) {
                const Listener listener = mListeners[uniqueEventID][index];
                if (!listener.callback) continue;
                reinterpret_cast<Callback<TEvent>>(listener.callback)(event, listener.context);
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

        [[nodiscard]] bool HasNoListeners() const noexcept {
            return std::ranges::all_of(mListeners, [](const std::vector<Listener>& listeners) {
                return std::ranges::none_of(listeners, [](const Listener& listener) { return listener.callback != nullptr; });
            });
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