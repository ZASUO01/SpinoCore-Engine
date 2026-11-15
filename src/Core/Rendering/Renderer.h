// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "SDL/Structs.h"

namespace SpinoCore::Core::Rendering {
    class Renderer final {
    public:
        explicit Renderer(SDL_Window* window);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        void Draw();

    private:
        SDL::UniqueGLContext mGLContext{nullptr};
        SDL_Window* mSDLWindow{nullptr};


    };
}