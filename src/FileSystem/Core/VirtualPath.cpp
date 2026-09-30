// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "VirtualPath.h"

namespace SpinoCore::FileSystem::Core {
    std::optional<VirtualPath> ParseVirtualPath(std::string_view fullVirtualPath){
        if (fullVirtualPath.empty()) {
            return std::nullopt;
        }

        constexpr std::string_view delimiter = "://";
        const size_t delimiterPos = fullVirtualPath.find(delimiter);

        if (delimiterPos == std::string_view::npos) {
            return std::nullopt;
        }

        if (delimiterPos == 0) {
            return std::nullopt;
        }

        const size_t pathStart = delimiterPos + delimiter.length();

        if (pathStart >= fullVirtualPath.length()) {
            return std::nullopt;
        }

        return VirtualPath{
            .protocol = fullVirtualPath.substr(0, delimiterPos),
            .path = fullVirtualPath.substr(pathStart)
        };
    }

    bool IsValidVirtualPath(const std::string_view virtualPath) {
        if (virtualPath.empty() || virtualPath.front() == '/' || virtualPath.back() == '/') {
            return false;
        }

        if (virtualPath.find("..") != std::string_view::npos) {
            return false;
        }

        bool lastWasSlash = false;

        for (const char c : virtualPath) {
            if (c == '/') {
                if (lastWasSlash) {
                    return false;
                }
                lastWasSlash = true;
                continue;
            }

            lastWasSlash = false;

            const bool isAlphaNumeric = (c >= 'a' && c <= 'z') ||
                                        (c >= 'A' && c <= 'Z') ||
                                        (c >= '0' && c <= '9');

            if (const bool isValidSymbol = c == '_' || c == '-' || c == '.'; !isAlphaNumeric && !isValidSymbol) {
                return false;
            }
        }

        return true;
    }
}