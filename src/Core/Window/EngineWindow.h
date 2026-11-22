// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <optional>
#include "SDL/Structs.h"
#include "SpinoCore/Core/Window/WindowManager.h"
#include "SpinoCore/Core/Window/WindowProps.h"
#include "SpinoCore/Core/Window/WindowSize.h"

namespace SpinoCore::Core::Window {
    class EngineWindow final : public WindowManager{
    public:
        explicit EngineWindow(const WindowProps& props);
        ~EngineWindow() override;

        EngineWindow(const EngineWindow&) = delete;
        EngineWindow& operator=(const EngineWindow&) = delete;
        EngineWindow(EngineWindow&&) = delete;
        EngineWindow& operator=(EngineWindow&&) = delete;

        [[nodiscard]] WindowMode GetMode() const noexcept override;
        bool SetMode(WindowMode mode) noexcept override;

        [[nodiscard]] Size2D GetSize() const noexcept override;
        bool SetSize(Size2D size) noexcept override;

        [[nodiscard]] static bool IsSystemCursorVisible() noexcept;
        static void SetSystemCursorVisible(bool visible) noexcept;

        [[nodiscard]] SDL_Window* GetSDLWindow() const { return mSDLWindow.get(); }

    private:
        SDL::UniqueSDLWindow mSDLWindow{nullptr};
        Size2D mWindowedSize{};
        std::optional<Size2D> mFullscreenSize{};
    };
}


