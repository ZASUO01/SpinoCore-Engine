#pragma once
#include "Config/Constants.h"
#include "nlohmann/json.hpp"

namespace SpinoCore::Config {
    class Store;
}

namespace SpinoCore::Config::Helpers {
    struct AppConfig {
        std::string name = Constants::App::NAME.data();
        std::string version = Constants::App::VERSION.data();
        std::string identifier = Constants::App::IDENTIFIER.data();
        std::string creator = Constants::App::CREATOR.data();
        std::string url = Constants::App::URL.data();
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(AppConfig, name, version, identifier, creator, url);

    struct WindowConfig {
        std::string title = Constants::Window::WINDOW_TITLE.data();
        int initialWidth = Constants::Window::INITIAL_WIDTH;
        int initialHeight = Constants::Window::INITIAL_HEIGHT;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(WindowConfig, title, initialWidth, initialHeight);

    struct RootConfig {
        uint8_t logLevel = Constants::Logs::LOG_LEVEL;
        AppConfig app;
        WindowConfig window;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(RootConfig, logLevel, app, window);

    void SetInitialConfig(const RootConfig& rootConfig, Store& store);
}