// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "FileHelpers.h"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <limits>
#include <new>
#include <system_error>
#include "SpinoCore/Logs/Logger.h"

namespace SpinoCore::Files {
    namespace {
        [[nodiscard]] std::filesystem::path TemporaryPathFor(const std::filesystem::path& filePath) {
            std::filesystem::path temporary = filePath;
            temporary += ".tmp";
            return temporary;
        }

        void RemoveQuietly(const std::filesystem::path& path) noexcept {
            std::error_code ignored;
            std::filesystem::remove(path, ignored);
        }
    }

    bool DirectoryExists(const std::filesystem::path& path) {
        std::error_code error;
        return std::filesystem::is_directory(path, error);
    }

    bool FileExists(const std::filesystem::path& path) {
        std::error_code error;
        return std::filesystem::is_regular_file(path, error);
    }

    std::optional<std::vector<std::uint8_t>> ReadFile(const std::filesystem::path& filePath) {
        if (!FileExists(filePath)) {
            Logs::Logger::Error("[FILES] Not a file: \"{}\"", filePath.string());
            return std::nullopt;
        }

        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file) {
            Logs::Logger::Error("[FILES] Could not open \"{}\"", filePath.string());
            return std::nullopt;
        }

        const std::streamoff size = file.tellg();
        if (size < 0 || static_cast<std::uintmax_t>(size) > std::numeric_limits<std::size_t>::max()) {
            Logs::Logger::Error("[FILES] Could not get the size of \"{}\"", filePath.string());
            return std::nullopt;
        }
        file.seekg(0, std::ios::beg);

        try {
            std::vector<std::uint8_t> data(static_cast<std::size_t>(size));
            if (!data.empty()) {
                file.read(reinterpret_cast<char*>(data.data()), size);
                if (file.gcount() != static_cast<std::streamsize>(size)) {
                    Logs::Logger::Error("[FILES] Could not read all of \"{}\" ({} of {} bytes)",
                                        filePath.string(), file.gcount(), size);
                    return std::nullopt;
                }
            }
            return data;
        } catch (const std::bad_alloc&) {
            Logs::Logger::Error("[FILES] Not enough memory to read \"{}\" ({} bytes)", filePath.string(), size);
            return std::nullopt;
        }
    }

    bool WriteFile(const std::filesystem::path& filePath, const std::span<const std::uint8_t> data) {
        if (filePath.empty() || !filePath.has_filename()) {
            Logs::Logger::Error("[FILES] Invalid file path: \"{}\"", filePath.string());
            return false;
        }

        std::error_code error;
        if (const auto parent = filePath.parent_path(); !parent.empty()) {
            std::filesystem::create_directories(parent, error);
            if (error) {
                Logs::Logger::Error("[FILES] Could not create \"{}\": {}", parent.string(), error.message());
                return false;
            }
        }

        const auto temporary = TemporaryPathFor(filePath);
        {
            std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
            if (!file) {
                Logs::Logger::Error("[FILES] Could not create \"{}\"", temporary.string());
                return false;
            }
            if (!data.empty()) {
                file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
            }

            file.close();
            if (!file) {
                RemoveQuietly(temporary);
                Logs::Logger::Error("[FILES] Could not write \"{}\"", temporary.string());
                return false;
            }
        }

        std::filesystem::rename(temporary, filePath, error);
        if (error) {
            RemoveQuietly(temporary);
            Logs::Logger::Error("[FILES] Could not replace \"{}\": {}", filePath.string(), error.message());
            return false;
        }
        return true;
    }
}
