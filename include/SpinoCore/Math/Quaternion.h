// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <type_traits>
#include "Vector.h"

namespace SpinoCore::Math {
    /// @brief Rotation stored as a quaternion.
    /// @tparam T Floating point type.
    template <typename T>
    class Quaternion final {
        static_assert(std::is_floating_point_v<T>, "Quaternion must be of floating point type.");
        constexpr Quaternion(Vector3<T> axis, T s, T c) noexcept
            : x(axis.x * s), y(axis.y * s), z(axis.z * s), w(c) {}
    public:
        /// X of the vector part.
        T x;
        /// Y of the vector part.
        T y;
        /// Z of the vector part.
        T z;
        /// Scalar part.
        T w;

        /// @brief Creates the identity rotation.
        constexpr Quaternion() noexcept : x(T{0}), y(T{0}), z(T{0}), w(T{1}) {}
        /// @brief Creates a quaternion from its components.
        /// @param inX X of the vector part.
        /// @param inY Y of the vector part.
        /// @param inZ Z of the vector part.
        /// @param inW Scalar part.
        constexpr Quaternion(T inX, T inY, T inZ, T inW) noexcept : x(inX), y(inY), z(inZ), w(inW) {}
        /// @brief Creates a rotation around an axis.
        /// @param normalizedAxis Unit rotation axis.
        /// @param angleRadians Angle in radians, counter-clockwise when looking down the axis.
        Quaternion(Vector3<T> normalizedAxis, T angleRadians) noexcept
            : Quaternion(
                normalizedAxis,
                Operations::Sin(angleRadians * T{0.5}),
                Operations::Cos(angleRadians * T{0.5})
              ) {}

        /// @return The squared length.
        [[nodiscard]] constexpr T LengthSq() const noexcept {
            return x * x + y * y + z * z + w * w;
        }

        /// @return The length.
        [[nodiscard]] T Length() const noexcept {
            return Operations::Sqrt<T>(LengthSq());
        }

        /// @brief Scales the quaternion to length 1; a zero quaternion is left unchanged.
        void Normalize() noexcept {
            const T lenSq = LengthSq();

            if (lenSq > Constants::Epsilon<T>) {
                const T invLen = T{1} / Operations::Sqrt<T>(lenSq);
                x *= invLen; y *= invLen; z *= invLen; w *= invLen;
            }
        }

        /// @return A unit quaternion with the same rotation.
        [[nodiscard]] Quaternion Normalized() const noexcept {
            Quaternion res = *this;
            res.Normalize();
            return res;
        }

        /// @return The quaternion with the vector part negated.
        [[nodiscard]] constexpr Quaternion GetConjugate() const noexcept {
            return Quaternion(-x, -y, -z, w);
        }

        /// @brief Negates the vector part in place.
        constexpr void Conjugate() noexcept {
            x = -x; y = -y; z = -z;
        }

        /// @param outSuccess Receives false when the quaternion is zero; it may be nullptr.
        /// @return The inverse, or the identity when there is none.
        [[nodiscard]] constexpr Quaternion GetInverse(bool* outSuccess = nullptr) const noexcept {
            const T lenSq = LengthSq();
            if (lenSq <= static_cast<T>(1e-8)) {
                if (outSuccess) *outSuccess = false;
                return Quaternion();
            }
            if (outSuccess) *outSuccess = true;

            const T invLenSq = T{1} / lenSq;
            return Quaternion(-x * invLenSq, -y * invLenSq, -z * invLenSq, w * invLenSq);
        }

        /// @brief Inverts the quaternion in place; a zero quaternion is left unchanged.
        constexpr void Invert() noexcept {
            bool success = false;
            const Quaternion inv = GetInverse(&success);
            if (success) {
                *this = inv;
            }
        }

        /// @param rhs Other quaternion.
        /// @return The 4D dot product.
        [[nodiscard]] constexpr T Dot(const Quaternion& rhs) const noexcept {
            return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
        }

        /// @return The rotation b followed by the rotation a.
        [[nodiscard]] friend constexpr Quaternion operator*(Quaternion a, Quaternion b) noexcept {
            return Quaternion(
                a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
                a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
                a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
                a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
            );
        }

        /// @brief Multiplies this quaternion by another on the right.
        constexpr Quaternion& operator*=(const Quaternion& rhs) noexcept {
            *this = *this * rhs;
            return *this;
        }

        /// @param start Rotation at t = 0.
        /// @param end Rotation at t = 1.
        /// @param t Interpolation factor.
        /// @return The normalized linear interpolation along the shortest path.
        [[nodiscard]] static Quaternion Lerp(const Quaternion& start, const Quaternion& end, T t) noexcept {
            T cosTheta = start.Dot(end);
            Quaternion result;

            if (cosTheta < T{0}) {
                result.x = start.x + (-end.x - start.x) * t;
                result.y = start.y + (-end.y - start.y) * t;
                result.z = start.z + (-end.z - start.z) * t;
                result.w = start.w + (-end.w - start.w) * t;
            } else {
                result.x = start.x + (end.x - start.x) * t;
                result.y = start.y + (end.y - start.y) * t;
                result.z = start.z + (end.z - start.z) * t;
                result.w = start.w + (end.w - start.w) * t;
            }

            return result.Normalized();
        }

        /// @param start Rotation at t = 0.
        /// @param end Rotation at t = 1.
        /// @param t Interpolation factor.
        /// @return The spherical interpolation along the shortest path.
        [[nodiscard]] static Quaternion Slerp(Quaternion start, Quaternion end, T t) noexcept {
            T cosTheta = start.Dot(end);
            Quaternion endTarget = end;

            if (cosTheta < T{0}) {
                cosTheta = -cosTheta;
                endTarget.x = -end.x;
                endTarget.y = -end.y;
                endTarget.z = -end.z;
                endTarget.w = -end.w;
            }

            if (cosTheta > static_cast<T>(0.99999)) {
                return Lerp(start, endTarget, t);
            }

            const T halfTheta = Operations::Acos(cosTheta);
            const T sinHalfTheta = Operations::Sqrt<T>(T{1} - cosTheta * cosTheta);

            const T invSinHalfTheta = T{1} / sinHalfTheta;
            const T weightStart = Operations::Sin((T{1} - t) * halfTheta) * invSinHalfTheta;
            const T weightEnd = Operations::Sin(t * halfTheta) * invSinHalfTheta;

            return Quaternion(
                start.x * weightStart + endTarget.x * weightEnd,
                start.y * weightStart + endTarget.y * weightEnd,
                start.z * weightStart + endTarget.z * weightEnd,
                start.w * weightStart + endTarget.w * weightEnd
            );
        }

        /// @brief Creates a rotation from Euler angles.
        /// @param pitch Rotation around X, in radians.
        /// @param yaw Rotation around Y, in radians.
        /// @param roll Rotation around Z, in radians.
        /// @return The rotation.
        [[nodiscard]] static Quaternion FromEuler(T pitch, T yaw, T roll) noexcept {
            const T halfPitch = pitch * T{0.5};
            const T halfYaw = yaw * T{0.5};
            const T halfRoll = roll * T{0.5};

            const T cx = Operations::Cos(halfPitch);
            const T sx = Operations::Sin(halfPitch);
            const T cy = Operations::Cos(halfYaw);
            const T sy = Operations::Sin(halfYaw);
            const T cz = Operations::Cos(halfRoll);
            const T sz = Operations::Sin(halfRoll);

            return Quaternion(
                cy * sx * cz + sy * cx * sz,
                sy * cx * cz - cy * sx * sz,
                cy * cx * sz - sy * sx * cz,
                cy * cx * cz + sy * sx * sz
            );
        }

        /// @brief Creates the rotation of an orthonormal basis.
        /// @param right Local X axis.
        /// @param up Local Y axis.
        /// @param back Local Z axis.
        /// @return The rotation.
        [[nodiscard]] static Quaternion FromAxes(const Vector3<T>& right, const Vector3<T>& up, const Vector3<T>& back) noexcept {
            const T trace = right.x + up.y + back.z;

            if (trace > T{0}) {
                const T s = Operations::Sqrt(trace + T{1}) * T{2};
                return Quaternion((up.z - back.y) / s, (back.x - right.z) / s, (right.y - up.x) / s, s / T{4});
            }
            if (right.x > up.y && right.x > back.z) {
                const T s = Operations::Sqrt(T{1} + right.x - up.y - back.z) * T{2};
                return Quaternion(s / T{4}, (up.x + right.y) / s, (back.x + right.z) / s, (up.z - back.y) / s);
            }
            if (up.y > back.z) {
                const T s = Operations::Sqrt(T{1} + up.y - right.x - back.z) * T{2};
                return Quaternion((up.x + right.y) / s, s / T{4}, (back.y + up.z) / s, (back.x - right.z) / s);
            }
            const T s = Operations::Sqrt(T{1} + back.z - right.x - up.y) * T{2};
            return Quaternion((back.x + right.z) / s, (back.y + up.z) / s, s / T{4}, (right.y - up.x) / s);
        }

        /// @brief Creates a rotation whose -Z axis points along a direction.
        /// @param forward Direction to face; a zero vector gives the identity.
        /// @param up Approximate up direction.
        /// @return The rotation.
        [[nodiscard]] static Quaternion LookRotation(const Vector3<T>& forward, const Vector3<T>& up = Vector3<T>::UnitY()) noexcept {
            const T forwardLengthSq = forward.LengthSq();
            if (forwardLengthSq <= T{0}) return Identity();

            const Vector3<T> back = -forward / Operations::Sqrt(forwardLengthSq);
            Vector3<T> right = up.Cross(back);
            if (right.LengthSq() <= static_cast<T>(1e-8)) {
                const Vector3<T> fallbackUp = Operations::Abs(back.y) > static_cast<T>(0.99) ? Vector3<T>::UnitZ() : Vector3<T>::UnitY();
                right = fallbackUp.Cross(back);
            }
            right.Normalize();

            return FromAxes(right, back.Cross(right), back);
        }

        /// @param v Vector to rotate.
        /// @return The rotated vector.
        [[nodiscard]] constexpr Vector3<T> Rotate(const Vector3<T>& v) const noexcept {
            const Vector3<T> axis{x, y, z};
            const Vector3<T> t = axis.Cross(v) * T{2};
            return v + t * w + axis.Cross(t);
        }

        /// @return The vector rotated by the quaternion.
        [[nodiscard]] friend constexpr Vector3<T> operator*(const Quaternion& q, const Vector3<T>& v) noexcept {
            return q.Rotate(v);
        }

        /// @return The identity rotation.
        [[nodiscard]] static constexpr Quaternion Identity() noexcept {
            return Quaternion();
        }

        /// @param a First rotation.
        /// @param b Second rotation.
        /// @param epsilon Tolerance per component.
        /// @return Whether both quaternions represent the same rotation, including q and -q.
        [[nodiscard]] static constexpr bool NearlyEqual(const Quaternion& a, const Quaternion& b,
                                                        T epsilon = Constants::Tolerance<T>) noexcept {
            const auto sameComponents = [epsilon](const Quaternion& lhs, const Quaternion& rhs) {
                return Operations::NearlyEqual(lhs.x, rhs.x, epsilon) && Operations::NearlyEqual(lhs.y, rhs.y, epsilon) &&
                       Operations::NearlyEqual(lhs.z, rhs.z, epsilon) && Operations::NearlyEqual(lhs.w, rhs.w, epsilon);
            };
            return sameComponents(a, b) || sameComponents(a, Quaternion(-b.x, -b.y, -b.z, -b.w));
        }
    };



    /// Float quaternion.
    using Quaternionf = Quaternion<float>;
}