// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <algorithm>
#include <cmath>
#include <concepts>
#include <limits>
#include <numbers>
#include <type_traits>

namespace SpinoCore::Math::Constants {
    template<typename T>
    constexpr T PI = std::numbers::pi_v<T>;

    template<typename T>
    constexpr T TwoPI = PI<T> * T{2};

    template<typename T>
    constexpr T PIOver2 = PI<T> / T{2};

    template<typename T>
    constexpr T PIOver4 = PI<T> / T{4};

    template<typename T>
    constexpr T DegToRad = PI<T> / T{180};

    template<typename T>
    constexpr T RadToDeg = T{180} / PI<T>;

    template<typename T>
    constexpr T Infinity = std::numeric_limits<T>::infinity();

    template<typename T>
    constexpr T NegInfinity = -std::numeric_limits<T>::infinity();

    template<typename T>
    constexpr T NaN = std::numeric_limits<T>::quiet_NaN();

    template<typename T>
    constexpr T Epsilon = std::numeric_limits<T>::epsilon();

    template<std::floating_point T>
    constexpr T Tolerance = static_cast<T>(1e-5);
}

namespace SpinoCore::Math::Operations {
    using std::swap;
    using std::max;
    using std::min;
    using std::clamp;
    using std::lerp;

    template <typename T>
    requires std::is_arithmetic_v<T>
    [[nodiscard]] constexpr T Abs(const T value) noexcept {
        if constexpr (std::is_unsigned_v<T>) {
            return value;
        } else {
            return value < T{0} ? -value : value;
        }
    }

    template <std::floating_point T>
    [[nodiscard]] T Sqrt(const T value) noexcept {
        return std::sqrt(value);
    }

    template <typename T>
    requires std::is_signed_v<T>
    [[nodiscard]] constexpr T Sgn(const T a) noexcept {
        return static_cast<T>((T{0} < a) - (a < T{0}));
    }

    template <std::floating_point T>
    [[nodiscard]] T Fmod(const T x, const T y) noexcept {
        return std::fmod(x, y);
    }

    template <std::floating_point T>
    [[nodiscard]] constexpr bool NearlyEqual(const T a, const T b, const T epsilon = Constants::Tolerance<T>) noexcept {
        const T diff = Abs(a - b);
        if (diff <= epsilon) return true;

        const T largest = max(Abs(a), Abs(b));
        return diff <= largest * epsilon;
    }

    template <std::floating_point T>
    [[nodiscard]] constexpr bool NearlyZero(const T value, const T epsilon = Constants::Tolerance<T>) noexcept {
        return Abs(value) <= epsilon;
    }

    template <std::floating_point T>
    [[nodiscard]] constexpr T ToRadians(const T degrees) noexcept {
        return degrees * Constants::DegToRad<T>;
    }

    template <std::floating_point T>
    [[nodiscard]] constexpr T ToDegrees(const T radians) noexcept {
        return radians * Constants::RadToDeg<T>;
    }

    template <std::floating_point T>
    [[nodiscard]] T Sin(const T angle) noexcept {
        return std::sin(angle);
    }

    template <std::floating_point T>
    [[nodiscard]] T Cos(const T angle) noexcept {
        return std::cos(angle);
    }

    template <std::floating_point T>
    [[nodiscard]] T Tan(const T angle) noexcept {
        return std::tan(angle);
    }

    template <std::floating_point T>
    [[nodiscard]] T Asin(const T value) noexcept {
        return std::asin(clamp(value, T{-1}, T{1}));
    }

    template <std::floating_point T>
    [[nodiscard]] T Acos(const T value) noexcept {
        return std::acos(clamp(value, T{-1}, T{1}));
    }

    template <std::floating_point T>
    [[nodiscard]] T Atan2(const T y, const T x) noexcept {
        return std::atan2(y, x);
    }
}
