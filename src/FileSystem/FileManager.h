// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include <optional>
#include "SpinoCore/FileSystem/FileLoader.h"
#include "nlohmann/json_fwd.hpp"

namespace SpinoCore::Core::Engine {
    struct CoreModules;
}

namespace SpinoCore::FileSystem {
    using json = nlohmann::json;

    class FileManager final : public FileLoader {
    public:
        class ConstructorKey { friend class FileManager; ConstructorKey() = default; };
        FileManager(ConstructorKey, Core::Engine::CoreModules& coreModules);
        ~FileManager() override;

        FileManager(const FileManager &) = delete;
        FileManager &operator=(const FileManager &) = delete;
        FileManager(FileManager &&) = delete;
        FileManager &operator=(FileManager &&) = delete;

        [[nodiscard]] static std::unique_ptr<FileManager> Create(Core::Engine::CoreModules& coreModules);

        void LoadFiles(std::string_view manifestPath) override;
        [[nodiscard]] bool LoadFilesSync(std::string_view manifestPath) const;

        [[nodiscard]] std::optional<json> LoadJSONFile(std::string_view manifestPath, const std::vector<std::string>& requiredFields = {}) const;
    private:
        [[nodiscard]] static bool IsValidFileManifest(const json& inJSON);

        Core::Engine::CoreModules& mCoreModules;
    };
}
