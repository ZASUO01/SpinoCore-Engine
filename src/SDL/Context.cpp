// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <format>
#include <stdexcept>
#include <string>
#include <SDL3/SDL.h>
#include "Context.h"
#include "SpinoCore/Game/GameInfo.h"

namespace SpinoCore::SDL {
    Context::Context(const Game::GameInfo &gameInfo) {
        if (SDL_WasInit(0) != 0) {
            throw std::logic_error("[SDL CONTEXT] SDL is already initialized. Only one Context may exist at a time.");
        }

        SDL_SetAppMetadata(gameInfo.name.c_str(), gameInfo.version.c_str(), gameInfo.identifier.c_str());
        SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_CREATOR_STRING, gameInfo.creator.c_str());
        SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_URL_STRING, gameInfo.url.c_str());

        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
            const std::string error = SDL_GetError();
            SDL_Quit();
            throw std::runtime_error(std::format("[SDL CONTEXT] Failed to initialize SDL: {}", error));
        }
    }

    Context::~Context() {
        SDL_Quit();
    }
}
