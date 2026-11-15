// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "EngineWindow.h"
#include <format>
#include <stdexcept>
#include "SDL/Helpers/Helpers.h"
#include "SDL3/SDL.h"
#include "WindowSizeHelpers.h"
#include "SpinoCore/Logs/Logger.h"

namespace SpinoCore::Core::Window {
    namespace {
        [[nodiscard]] constexpr std::string_view ModeName(const WindowMode mode) noexcept {
            switch (mode) {
                case WindowMode::Windowed: return "windowed";
                case WindowMode::BorderlessFullscreen: return "borderless fullscreen";
                case WindowMode::Fullscreen: return "fullscreen";
            }
            return "unknown";
        }

        [[nodiscard]] std::optional<Size2D> UsableArea(const SDL_DisplayID display) noexcept {
            SDL_Rect bounds;
            if (display == 0 || !SDL_GetDisplayUsableBounds(display, &bounds)) return std::nullopt;
            return Size2D{.width = bounds.w, .height = bounds.h};
        }

        [[nodiscard]] bool ApplyExactFullscreenMode(SDL_Window* window, const Size2D resolution) noexcept {
            SDL_DisplayMode displayMode;
            return SDL_GetClosestFullscreenDisplayMode(SDL_GetDisplayForWindow(window),
                       resolution.width, resolution.height, 0.0f, false, &displayMode)
                && displayMode.w == resolution.width
                && displayMode.h == resolution.height
                && SDL_SetWindowFullscreenMode(window, &displayMode);
        }
    }

    EngineWindow::EngineWindow(const WindowProps &props) {
        if (props.title.empty()) {
            throw std::invalid_argument("[ENGINE WINDOW] Window title must not be empty.");
        }

        const auto currentDisplay = SDL_GetPrimaryDisplay();
        const auto usableArea = UsableArea(currentDisplay);
        const Size2D available = usableArea.value_or(WINDOW_SIZE_MIN);

        mWindowedSize = FitStandardWindowSize(props.size, available);
        if (mWindowedSize != props.size) {
            Logs::Logger::Warn("[ENGINE WINDOW] Window size {}x{} is not available, using {}x{} instead.",
                props.size.width, props.size.height, mWindowedSize.width, mWindowedSize.height);
        }

        mSDLWindow = SDL::Helpers::CreateSDLWindow(props.title.c_str(), mWindowedSize.width, mWindowedSize.height);
        if (!mSDLWindow) {
            throw std::runtime_error(std::format("[ENGINE WINDOW] Could not create window: {}", SDL_GetError()));
        }

        if (props.windowMode != WindowMode::Windowed) {
            SetMode(props.windowMode);
        }
    }

    EngineWindow::~EngineWindow() = default;

    WindowMode EngineWindow::GetMode() const noexcept {
        SDL_Window* const window = mSDLWindow.get();
        if (!(SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN)) return WindowMode::Windowed;

        return SDL_GetWindowFullscreenMode(window) ? WindowMode::Fullscreen : WindowMode::BorderlessFullscreen;
    }

    bool EngineWindow::SetMode(const WindowMode mode) noexcept {
        SDL_Window* const window = mSDLWindow.get();
        bool switched = false;

        switch (mode) {
            case WindowMode::Windowed:
                switched = SDL_SetWindowFullscreen(window, false)
                    && SDL_SetWindowSize(window, mWindowedSize.width, mWindowedSize.height);
                break;
            case WindowMode::BorderlessFullscreen:
                switched = SDL_SetWindowFullscreenMode(window, nullptr) && SDL_SetWindowFullscreen(window, true);
                break;
            case WindowMode::Fullscreen: {
                const SDL_DisplayMode* desktopMode = SDL_GetDesktopDisplayMode(SDL_GetDisplayForWindow(window));
                if (!desktopMode) break;

                const Size2D desktop{.width = desktopMode->w, .height = desktopMode->h};
                switched = ApplyExactFullscreenMode(window, mFullscreenSize.value_or(desktop))
                    && SDL_SetWindowFullscreen(window, true);
                break;
            }
        }

        SDL_SyncWindow(window);

        if (!switched) {
            Logs::Logger::Warn("[ENGINE WINDOW] Could not switch to {} mode: {}", ModeName(mode), SDL_GetError());
        }
        return switched;
    }

    Size2D EngineWindow::GetSize() const noexcept {
        Size2D size{};
        SDL_GetWindowSize(mSDLWindow.get(), &size.width, &size.height);
        return size;
    }

    bool EngineWindow::SetSize(const Size2D size) noexcept {
        if (!IsStandardWindowSize(size)) {
            Logs::Logger::Warn("[ENGINE WINDOW] {}x{} is not an allowed window size.", size.width, size.height);
            return false;
        }

        SDL_Window* const window = mSDLWindow.get();
        const WindowMode mode = GetMode();
        bool applied = false;

        if (mode == WindowMode::Fullscreen) {
            applied = ApplyExactFullscreenMode(window, size);
        } else {
            const auto available = UsableArea(SDL_GetDisplayForWindow(window));
            applied = available && Fits(size, *available);
            if (applied && mode == WindowMode::Windowed) {
                applied = SDL_SetWindowSize(window, size.width, size.height);
                if (applied) SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            }
        }

        SDL_SyncWindow(window);

        if (!applied) {
            Logs::Logger::Warn("[ENGINE WINDOW] Could not apply window size {}x{} in {} mode.",
                size.width, size.height, ModeName(mode));
            return false;
        }

        if (mode == WindowMode::Fullscreen) {
            mFullscreenSize = size;
        } else {
            mWindowedSize = size;
        }
        return true;
    }
}
