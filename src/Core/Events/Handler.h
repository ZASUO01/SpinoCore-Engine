#pragma once

namespace SpinoCore::Core::Events {
    class EventBus;

    class Handler final {
    public:
        Handler() = delete;

        [[nodiscard]] static bool PollEvents(EventBus& bus);
    };
}
