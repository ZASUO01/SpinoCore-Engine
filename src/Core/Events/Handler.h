#pragma once

namespace SpinoCore::Core::Events {
    class EventBus;
}

namespace SpinoCore::Core::Input {
    class InputState;
}

namespace SpinoCore::Core::Events {
    class Handler final {
    public:
        Handler() = delete;

        [[nodiscard]] static bool PollEvents(EventBus& bus, Input::InputState& input);
    };
}
