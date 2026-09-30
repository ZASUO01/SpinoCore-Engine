// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "FileHelpers.h"
#include <fstream>
#include "SpinoCore/Logs/Logger.h"

namespace SpinoCore::FileSystem::Helpers {
    std::optional<std::vector<uint8_t>> ReadFile(const std::filesystem::path& path, uint64_t offset, uint64_t sizeToRead) {
        std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);

        if (!file.is_open() || !file.good() ) {
            Logs::Logger::Error("[FILE HELPERS] Failed to open file '{}' for reading.", path.string());
            return std::nullopt;
        }

        const auto fileSize = file.tellg();
        if (fileSize < 0) {
            Logs::Logger::Error("[FILE HELPERS] Failed to get the size of file '{}'.", path.string());
            return std::nullopt;
        }

        const uint64_t totalSize = fileSize;
        if (offset >= totalSize) {
            Logs::Logger::Warn("[FILE HELPERS] The offset ({}) is greater or equal to the size ({}) of the file '{}'. Returning an empty buffer.",
                offset, totalSize, path.string());
            return std::vector<uint8_t>{};
        }

        const uint64_t availableBytes = totalSize - offset;
        const uint64_t bytesToRead = sizeToRead == READ_ALL || sizeToRead > availableBytes ? availableBytes : sizeToRead;

        if (bytesToRead == 0) {
            Logs::Logger::Warn("[FILE HELPERS] 0 bytes to read on file '{}'. Returning an empty buffer.", path.string());
            return std::vector<uint8_t>{};
        }

        auto rawBuffer = std::make_unique_for_overwrite<uint8_t[]>(bytesToRead);

        file.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
        if (!file.good()) {
            Logs::Logger::Error("[FILE HELPERS] Seek failure at offset {} in file '{}'.",offset, path.string());
            return std::nullopt;
        }

        if (file.read(reinterpret_cast<char*>(rawBuffer.get()), static_cast<std::streamsize>(bytesToRead))) {
            return std::vector(rawBuffer.get(), rawBuffer.get() + bytesToRead);
        }

        Logs::Logger::Error("[FILE HELPERS] Failed to read bytes from file '{}'.", path.string());
        return std::nullopt;
    }

    bool WriteToFile(const std::filesystem::path& path, const std::vector<uint8_t>& data, const uint64_t offset) {
        std::ios_base::openmode mode = std::ios::out | std::ios::binary;

        if (offset != IGNORE_OFFSET && std::filesystem::exists(path)) {
            mode |= std::ios::in;
        } else {
            mode |= std::ios::trunc;
        }

        std::ofstream file(path, mode);
        if (!file.is_open() || !file.good()) {
            Logs::Logger::Error("[FILE HELPERS] Failed to open file '{}' for writing.", path.string());
            return false;
        }

        if (offset != IGNORE_OFFSET) {
            file.seekp(static_cast<std::streamoff>(offset), std::ios::beg);
            if (!file.good()) {
                Logs::Logger::Error("[FILE HELPERS] Seek failure at offset {} in file '{}'.", offset, path.string());
                return false;
            }
        }

        if (data.empty()) {
            Logs::Logger::Warn("[FILE HELPERS] Empty data buffer. Nothing to write on file '{}'", path.string());
            return true;
        }

        file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
        if (!file.good()) {
            Logs::Logger::Error("[FILE HELPERS] Failed while writing on file '{}'.", path.string());
            return false;
        }

        return true;
    }
}
