// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <concepts>
#include <type_traits>
#include "Matrix.h"
#include "Quaternion.h"
#include "Vector.h"

namespace SpinoCore::Math::MatrixFactory {
    template <typename T>
    requires std::is_arithmetic_v<T>
    [[nodiscard]] constexpr Vector3<T> GetRight(const Matrix4<T>& mat) noexcept {
        return Vector3<T>{mat(0, 0), mat(1, 0), mat(2, 0)};
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    [[nodiscard]] constexpr Vector3<T> GetUp(const Matrix4<T>& mat) noexcept {
        return Vector3<T>{mat(0, 1), mat(1, 1), mat(2, 1)};
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    [[nodiscard]] constexpr Vector3<T> GetForward(const Matrix4<T>& mat) noexcept {
        return Vector3<T>{-mat(0, 2), -mat(1, 2), -mat(2, 2)};
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    [[nodiscard]] constexpr Vector3<T> GetTranslation(const Matrix4<T>& mat) noexcept {
        return Vector3<T>{mat(0, 3), mat(1, 3), mat(2, 3)};
    }

    template <std::floating_point T>
    [[nodiscard]] Vector3<T> GetRotationEuler(const Matrix4<T>& mat) noexcept {
        const Vector3<T> right = GetRight(mat).Normalized();
        const Vector3<T> up = GetUp(mat).Normalized();
        const Vector3<T> back = (-GetForward(mat)).Normalized();

        Vector3<T> euler;

        const T sinPitch = -back.y;
        euler.x = Operations::Asin(sinPitch);

        if (Operations::Abs(sinPitch) < static_cast<T>(0.9999)) {
            euler.y = Operations::Atan2(back.x, back.z);
            euler.z = Operations::Atan2(right.y, up.y);
        } else {
            euler.y = Operations::Atan2(-right.z, right.x);
            euler.z = T{0};
        }

        return euler;
    }

    template <std::floating_point T>
    [[nodiscard]] Vector3<T> GetScale(const Matrix4<T>& mat) noexcept {
        return Vector3<T>{
            GetRight(mat).Length(),
            GetUp(mat).Length(),
            (-GetForward(mat)).Length()
        };
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    [[nodiscard]] constexpr Matrix4<T> CreateTranslation(const Vector3<T>& translation) noexcept {
        Matrix4<T> mat;
        mat(0, 3) = translation.x;
        mat(1, 3) = translation.y;
        mat(2, 3) = translation.z;
        return mat;
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    [[nodiscard]] constexpr Matrix4<T> CreateScale(const T xScale, const T yScale, const T zScale) noexcept {
        Matrix4<T> mat;
        mat(0, 0) = xScale;
        mat(1, 1) = yScale;
        mat(2, 2) = zScale;
        return mat;
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    [[nodiscard]] constexpr Matrix4<T> CreateScale(const Vector3<T>& scale) noexcept {
        return CreateScale(scale.x, scale.y, scale.z);
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    [[nodiscard]] constexpr Matrix4<T> CreateScale(const T scale) noexcept {
        return CreateScale(scale, scale, scale);
    }

    template <std::floating_point T>
    [[nodiscard]] Matrix4<T> CreateRotationX(const T angleRadians) noexcept {
        Matrix4<T> mat;
        const T c = Operations::Cos(angleRadians);
        const T s = Operations::Sin(angleRadians);

        mat(1, 1) = c;  mat(1, 2) = -s;
        mat(2, 1) = s;  mat(2, 2) = c;

        return mat;
    }

    template <std::floating_point T>
    [[nodiscard]] Matrix4<T> CreateRotationY(const T angleRadians) noexcept {
        Matrix4<T> mat;
        const T c = Operations::Cos(angleRadians);
        const T s = Operations::Sin(angleRadians);

        mat(0, 0) = c;   mat(0, 2) = s;
        mat(2, 0) = -s;  mat(2, 2) = c;

        return mat;
    }

    template <std::floating_point T>
    [[nodiscard]] Matrix4<T> CreateRotationZ(const T angleRadians) noexcept {
        Matrix4<T> mat;
        const T c = Operations::Cos(angleRadians);
        const T s = Operations::Sin(angleRadians);

        mat(0, 0) = c;  mat(0, 1) = -s;
        mat(1, 0) = s;  mat(1, 1) = c;

        return mat;
    }

    template <std::floating_point T>
    [[nodiscard]] constexpr Matrix4<T> CreateFromQuaternion(const Quaternion<T>& q) noexcept {
        Matrix4<T> mat;

        const T xx = q.x * q.x;
        const T yy = q.y * q.y;
        const T zz = q.z * q.z;

        const T xy = q.x * q.y;
        const T xz = q.x * q.z;
        const T yz = q.y * q.z;

        const T wx = q.w * q.x;
        const T wy = q.w * q.y;
        const T wz = q.w * q.z;

        mat(0, 0) = T{1} - T{2} * (yy + zz);
        mat(1, 0) = T{2} * (xy + wz);
        mat(2, 0) = T{2} * (xz - wy);

        mat(0, 1) = T{2} * (xy - wz);
        mat(1, 1) = T{1} - T{2} * (xx + zz);
        mat(2, 1) = T{2} * (yz + wx);

        mat(0, 2) = T{2} * (xz + wy);
        mat(1, 2) = T{2} * (yz - wx);
        mat(2, 2) = T{1} - T{2} * (xx + yy);

        return mat;
    }

    template <std::floating_point T>
    [[nodiscard]] constexpr Matrix4<T> CreateTransform(const Vector3<T>& translation, const Quaternion<T>& rotation,
                                                       const Vector3<T>& scale) noexcept {
        return CreateTranslation(translation) * CreateFromQuaternion(rotation) * CreateScale(scale);
    }

    template <std::floating_point T>
    [[nodiscard]] Matrix4<T> CreateLookAt(const Vector3<T>& eye, const Vector3<T>& target, const Vector3<T>& up) noexcept {
        Vector3<T> forward = target - eye;
        const T forwardLengthSq = forward.LengthSq();
        if (forwardLengthSq < static_cast<T>(1e-8)) return Matrix4<T>();
        forward /= Operations::Sqrt(forwardLengthSq);

        Vector3<T> right = forward.Cross(up);
        const T rightLengthSq = right.LengthSq();
        if (rightLengthSq < static_cast<T>(1e-8)) {
            const Vector3<T> fallbackUp = Operations::Abs(forward.y) > static_cast<T>(0.99) ? Vector3<T>::UnitZ() : Vector3<T>::UnitY();
            right = forward.Cross(fallbackUp).Normalized();
        } else {
            right /= Operations::Sqrt(rightLengthSq);
        }

        const Vector3<T> trueUp = right.Cross(forward);

        Matrix4<T> mat;

        mat(0, 0) = right.x;
        mat(0, 1) = right.y;
        mat(0, 2) = right.z;
        mat(0, 3) = -right.Dot(eye);

        mat(1, 0) = trueUp.x;
        mat(1, 1) = trueUp.y;
        mat(1, 2) = trueUp.z;
        mat(1, 3) = -trueUp.Dot(eye);

        mat(2, 0) = -forward.x;
        mat(2, 1) = -forward.y;
        mat(2, 2) = -forward.z;
        mat(2, 3) = forward.Dot(eye);

        return mat;
    }

    template <std::floating_point T>
    [[nodiscard]] constexpr Matrix4<T> CreateOrtho(const T width, const T height, const T nearPlane, const T farPlane) noexcept {
        Matrix4<T> mat;

        if (width <= T{0} || height <= T{0} || nearPlane == farPlane) return mat;

        mat(0, 0) = T{2} / width;
        mat(1, 1) = T{2} / height;
        mat(2, 2) = T{-2} / (farPlane - nearPlane);
        mat(2, 3) = -(farPlane + nearPlane) / (farPlane - nearPlane);

        return mat;
    }

    template <std::floating_point T>
    [[nodiscard]] constexpr Matrix4<T> CreateOrtho(const T left, const T right, const T bottom, const T top,
                                                   const T nearPlane, const T farPlane) noexcept {
        Matrix4<T> mat;

        if (left == right || top == bottom || nearPlane == farPlane) return mat;

        mat(0, 0) = T{2} / (right - left);
        mat(1, 1) = T{2} / (top - bottom);
        mat(2, 2) = T{-2} / (farPlane - nearPlane);

        mat(0, 3) = -(right + left) / (right - left);
        mat(1, 3) = -(top + bottom) / (top - bottom);
        mat(2, 3) = -(farPlane + nearPlane) / (farPlane - nearPlane);

        return mat;
    }

    template <std::floating_point T>
    [[nodiscard]] Matrix4<T> CreatePerspectiveFOV(const T fovYRadians, const T width, const T height,
                                                  const T nearPlane, const T farPlane) noexcept {
        Matrix4<T> mat;

        if (width <= T{0} || height <= T{0} || nearPlane == farPlane) return mat;

        const T yScale = T{1} / Operations::Tan(fovYRadians * T{0.5});
        const T xScale = yScale * height / width;

        mat(0, 0) = xScale;
        mat(1, 1) = yScale;
        mat(2, 2) = (farPlane + nearPlane) / (nearPlane - farPlane);
        mat(2, 3) = T{2} * farPlane * nearPlane / (nearPlane - farPlane);
        mat(3, 2) = T{-1};
        mat(3, 3) = T{0};

        return mat;
    }
}
