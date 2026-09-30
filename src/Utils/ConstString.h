// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstddef>
#include <string_view>

namespace SpinoCore::Utils {
    template <std::size_t N>
    struct ConstString {
        char buffer[N]{};

        explicit constexpr ConstString(const char (&str)[N]) {
            for (std::size_t i = 0; i < N; ++i) {
                buffer[i] = str[i];
            }
        }

        explicit constexpr operator std::string_view() const {
            return {buffer, N - 1};
        }

        [[nodiscard]] constexpr const char* c_str() const {
            return buffer;
        }

        [[nodiscard]] constexpr const char* data() const { return buffer; }
        [[nodiscard]] static constexpr std::size_t size() { return N - 1; }

        [[nodiscard]] constexpr const char* begin() const { return buffer; }
        [[nodiscard]] constexpr const char* end() const { return buffer + N - 1; }
    };

    template <std::size_t N>
    ConstString(const char (&)[N]) -> ConstString<N>;

    template <std::size_t N1, std::size_t N2>
    constexpr ConstString<N1 + N2 - 1> operator+(const ConstString<N1>& lhs, const ConstString<N2>& rhs) {
        char tempBuf[N1 + N2 - 1]{};

        for (std::size_t i = 0; i < N1 - 1; ++i) {
            tempBuf[i] = lhs.buffer[i];
        }

        for (std::size_t i = 0; i < N2; ++i) {
            tempBuf[i + N1 - 1] = rhs.buffer[i];
        }

        return ConstString<N1 + N2 - 1>(tempBuf);
    }
}