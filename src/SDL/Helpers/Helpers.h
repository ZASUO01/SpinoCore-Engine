// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "SDL/Structs.h"

namespace SpinoCore::SDL::Helpers {
    [[nodiscard]] UniqueSDLWindow CreateSDLWindow(const char* title, int width, int height) noexcept;
    [[nodiscard]] UniqueGLContext CreateGLContext(SDL_Window* window) noexcept;
}
