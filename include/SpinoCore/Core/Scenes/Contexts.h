// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once

namespace SpinoCore::Core::Actors {
    class ActorsManager;
}

namespace SpinoCore::Core::Input {
    class InputState;
}

namespace SpinoCore::Files {
    class FileManager;
}

namespace SpinoCore::Core::Cameras {
    class CamerasManager;
}

namespace SpinoCore::Core::UI {
    class ScreensManager;
}

namespace SpinoCore::Core::Window {
    class WindowManager;
}

namespace SpinoCore::Core::Scenes {
    struct StartContext final {
        Files::FileManager& fileManager;
    };

    struct ControlContext final {
        const Files::FileManager& fileManager;
        Cameras::CamerasManager& camerasManager;
        UI::ScreensManager& screensManager;
        Window::WindowManager& windowManager;
        Actors::ActorsManager& actorsManager;
        const Input::InputState& simulationInput;
    };
}
