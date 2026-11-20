#pragma once

namespace SpinoCore::Core::Events {
    class Handler final {
    public:
        Handler() = delete;

        [[nodiscard]] static bool PollEvents();
    };
}
