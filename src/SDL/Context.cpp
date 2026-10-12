// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Context.h"
#include <SDL3/SDL.h>
#include <optional>

namespace SpinoCore::SDL {
    Context::Context() : mOwnsSDL(true) {}

    Context::Context(Context&& other) noexcept : mOwnsSDL(other.mOwnsSDL) {
        other.mOwnsSDL = false;
    }

    Context& Context::operator=(Context&& other) noexcept {
        if (this != &other) {
            if (mOwnsSDL) {
                SDL_Quit();
            }
            mOwnsSDL = other.mOwnsSDL;
            other.mOwnsSDL = false;
        }
        return *this;
    }

    Context::~Context() {
        if (mOwnsSDL) {
            SDL_Quit();
        }
    }

    std::optional<Context> Context::Create(const AppMetadata& metadata) {
        if (!Initialize(metadata)) return std::nullopt;

        return Context{};
    }

    bool Context::Initialize(const AppMetadata& metadata) {
        SDL_SetAppMetadata(metadata.appName.c_str(), metadata.appVersion.c_str(), metadata.appIdentifier.c_str());
        SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_CREATOR_STRING, metadata.appCreator.c_str());
        SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_URL_STRING, metadata.appURL.c_str());

        if (!SDL_IsMainThread()) {
            //Logs::Logger::Error("[SDL CONTEXT] Initialization of SDL Subsystems must be performed on main thread.");
            return false;
        }

        if (!SDL_Init(SDL_INIT_VIDEO)) {
            //Logs::Logger::Error("[SDL CONTEXT] Failed to initialize SDL: {}", SDL_GetError());
            return false;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

        return true;
    }
}