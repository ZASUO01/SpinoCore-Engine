// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once

/// @namespace SpinoCore::Core
/// @brief Engine core: actors, cameras, input, lighting, scenes, UI and window.
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

/// @brief Scenes and the contexts that give them access to the engine modules.
namespace SpinoCore::Core::Scenes {
    /// @brief Modules available to Scene::OnStart.
    struct StartContext final {
        /// Used to request the assets of the scene.
        Files::FileManager& fileManager;
    };

    /// @brief Modules available to Scene::OnSetup, OnUpdate and OnFinish.
    ///
    /// The references stay valid while the scene is on the stack.
    struct ControlContext final {
        /// Read-only file manager, for example to check failed loads.
        const Files::FileManager& fileManager;
        /// Scene cameras.
        Cameras::CamerasManager& camerasManager;
        /// Screen stack.
        UI::ScreensManager& screensManager;
        /// Window control.
        Window::WindowManager& windowManager;
        /// Actors of the scene.
        Actors::ActorsManager& actorsManager;
        /// Global light and shadows.
        Lighting::GlobalLight& globalLight;
        /// Post-processing effects.
        Graphics::PostProcessManager& postProcessManager;
        /// Synthesizer instruments.
        Sound::Synth::SynthManager& synthManager;
        /// Sound clips.
        Sound::SoundManager& soundManager;
        /// Input of the current simulation step.
        const Input::InputState& simulationInput;
    };
}
