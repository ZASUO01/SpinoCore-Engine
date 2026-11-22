// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once

namespace SpinoCore::Core::Actors {
    class ActorsManager;
}

namespace SpinoCore::Core::Lighting {
    class GlobalLight;
}

namespace SpinoCore::Core::Input {
    class InputState;
}

namespace SpinoCore::Graphics {
    class PostProcessManager;
}

namespace SpinoCore::Sound {
    class SoundManager;
}

namespace SpinoCore::Sound::Synth {
    class SynthManager;
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
        Lighting::GlobalLight& globalLight;
        Graphics::PostProcessManager& postProcessManager;
        Sound::Synth::SynthManager& synthManager;
        Sound::SoundManager& soundManager;
        const Input::InputState& simulationInput;
    };
}
