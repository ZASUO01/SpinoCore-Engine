// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <algorithm>
#include <cmath>
#include <concepts>
#include <limits>
#include <numbers>
#include <type_traits>

/// @brief Math constants for each floating point type.
namespace SpinoCore::Math::Constants {
    /// Pi.
    /// @tparam T Value type.
    template<typename T>
    constexpr T PI = std::numbers::pi_v<T>;

    /// Two times pi, a full turn in radians.
    /// @tparam T Value type.
    template<typename T>
    constexpr T TwoPI = PI<T> * T{2};

    /// Pi over 2, a quarter turn.
    /// @tparam T Value type.
    template<typename T>
    constexpr T PIOver2 = PI<T> / T{2};

    /// Pi over 4, an eighth of a turn.
    /// @tparam T Value type.
    template<typename T>
    constexpr T PIOver4 = PI<T> / T{4};

    /// Multiply degrees by it to get radians.
    /// @tparam T Value type.
    template<typename T>
    constexpr T DegToRad = PI<T> / T{180};

    /// Multiply radians by it to get degrees.
    /// @tparam T Value type.
    template<typename T>
    constexpr T RadToDeg = T{180} / PI<T>;

    /// Positive infinity.
    /// @tparam T Value type.
    template<typename T>
    constexpr T Infinity = std::numeric_limits<T>::infinity();

    /// Negative infinity.
    /// @tparam T Value type.
    template<typename T>
    constexpr T NegInfinity = -std::numeric_limits<T>::infinity();

    /// Quiet not-a-number.
    /// @tparam T Value type.
    template<typename T>
    constexpr T NaN = std::numeric_limits<T>::quiet_NaN();

    /// Machine epsilon of the type.
    /// @tparam T Value type.
    template<typename T>
    constexpr T Epsilon = std::numeric_limits<T>::epsilon();

    /// Default tolerance of the NearlyEqual comparisons.
    /// @tparam T Value type.
    template<std::floating_point T>
    constexpr T Tolerance = static_cast<T>(1e-5);
}

/// @brief Scalar math functions used by the engine.
namespace SpinoCore::Math::Operations {
    using std::swap;
    using std::max;
    using std::min;
    using std::clamp;
    using std::lerp;

    /// @brief Absolute value.
    /// @param value Input.
    /// @return The value without its sign.
    template <typename T>
    requires std::is_arithmetic_v<T>
    [[nodiscard]] constexpr T Abs(const T value) noexcept {
        if constexpr (std::is_unsigned_v<T>) {
            return value;
        } else {
            return value < T{0} ? -value : value;
        }
    }

    /// @brief Square root.
    /// @param value Non-negative input.
    /// @return The square root.
    template <std::floating_point T>
    [[nodiscard]] T Sqrt(const T value) noexcept {
        return std::sqrt(value);
    }

    /// @brief Sign.
    /// @param a Input.
    /// @return -1, 0 or 1.
    template <typename T>
    requires std::is_signed_v<T>
    [[nodiscard]] constexpr T Sgn(const T a) noexcept {
        return static_cast<T>((T{0} < a) - (a < T{0}));
    }

    /// @brief Floating point remainder.
    /// @param x Dividend.
    /// @param y Divisor.
    /// @return The remainder with the sign of x.
    template <std::floating_point T>
    [[nodiscard]] T Fmod(const T x, const T y) noexcept {
        return std::fmod(x, y);
    }

    /// @brief Compares with an absolute and a relative tolerance.
    /// @param a First value.
    /// @param b Second value.
    /// @param epsilon Tolerance.
    /// @return Whether the values are close.
    template <std::floating_point T>
    [[nodiscard]] constexpr bool NearlyEqual(const T a, const T b, const T epsilon = Constants::Tolerance<T>) noexcept {
        const T diff = Abs(a - b);
        if (diff <= epsilon) return true;

        const T largest = max(Abs(a), Abs(b));
        return diff <= largest * epsilon;
    }

    /// @brief Checks if a value is close to zero.
    /// @param value Input.
    /// @param epsilon Tolerance.
    /// @return Whether the absolute value is at most epsilon.
    template <std::floating_point T>
    [[nodiscard]] constexpr bool NearlyZero(const T value, const T epsilon = Constants::Tolerance<T>) noexcept {
        return Abs(value) <= epsilon;
    }

    /// @brief Converts degrees to radians.
    /// @param degrees Angle in degrees.
    /// @return The angle in radians.
    template <std::floating_point T>
    [[nodiscard]] constexpr T ToRadians(const T degrees) noexcept {
        return degrees * Constants::DegToRad<T>;
    }

    /// @brief Converts radians to degrees.
    /// @param radians Angle in radians.
    /// @return The angle in degrees.
    template <std::floating_point T>
    [[nodiscard]] constexpr T ToDegrees(const T radians) noexcept {
        return radians * Constants::RadToDeg<T>;
    }

    /// @brief Sine.
    /// @param angle Angle in radians.
    /// @return The sine.
    template <std::floating_point T>
    [[nodiscard]] T Sin(const T angle) noexcept {
        return std::sin(angle);
    }

    /// @brief Cosine.
    /// @param angle Angle in radians.
    /// @return The cosine.
    template <std::floating_point T>
    [[nodiscard]] T Cos(const T angle) noexcept {
        return std::cos(angle);
    }

    /// @brief Tangent.
    /// @param angle Angle in radians.
    /// @return The tangent.
    template <std::floating_point T>
    [[nodiscard]] T Tan(const T angle) noexcept {
        return std::tan(angle);
    }

    /// @brief Arc sine; the input is clamped to [-1, 1].
    /// @param value Input.
    /// @return The angle in radians.
    template <std::floating_point T>
    [[nodiscard]] T Asin(const T value) noexcept {
        return std::asin(clamp(value, T{-1}, T{1}));
    }

    /// @brief Arc cosine; the input is clamped to [-1, 1].
    /// @param value Input.
    /// @return The angle in radians.
    template <std::floating_point T>
    [[nodiscard]] T Acos(const T value) noexcept {
        return std::acos(clamp(value, T{-1}, T{1}));
    }

    /// @brief Angle of a 2D direction.
    /// @param y Y component.
    /// @param x X component.
    /// @return The angle in radians, in [-pi, pi].
    template <std::floating_point T>
    [[nodiscard]] T Atan2(const T y, const T x) noexcept {
        return std::atan2(y, x);
    }
}
