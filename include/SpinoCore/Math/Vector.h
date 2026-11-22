// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <concepts>
#include <type_traits>
#include "SpinoCore/Math/Math.h"

/// @brief Math types and functions: vectors, matrices, quaternions and helpers.
namespace SpinoCore::Math {
    template<typename T>
    /// @brief 2-component vector.
    /// @tparam T Arithmetic type of the components.
    class Vector2 final {
        static_assert(std::is_arithmetic_v<T>, "Vector2 must be arithmetic.");
    public:
        /// X component.
        T x;
        /// Y component.
        T y;

        /// @brief Creates a zero vector.
        constexpr Vector2() noexcept : x(T{}), y(T{}) {}
        /// @brief Creates a vector from its components.
        /// @param inX X component.
        /// @param inY Y component.
        constexpr Vector2(T inX, T inY) noexcept : x(inX), y(inY) {}
        /// @brief Creates a vector with every component equal.
        /// @param xy Value of every component.
        explicit constexpr Vector2(T xy) noexcept : x(xy), y(xy) {}

        /// @return A pointer to the contiguous components, for GPU uploads.
        [[nodiscard]] constexpr const T* Data() const noexcept { return &x; }
        /// @return A pointer to the contiguous components.
        [[nodiscard]] constexpr T* Data() noexcept { return &x; }

        /// @brief Adds another vector component-wise.
        constexpr Vector2& operator+=(Vector2 rhs) noexcept { x += rhs.x; y += rhs.y; return *this; }
        /// @brief Subtracts another vector component-wise.
        constexpr Vector2& operator-=(Vector2 rhs) noexcept { x -= rhs.x; y -= rhs.y; return *this; }
        /// @brief Multiplies by another vector component-wise.
        constexpr Vector2& operator*=(Vector2 rhs) noexcept { x *= rhs.x; y *= rhs.y; return *this; }
        /// @brief Multiplies every component by a scalar.
        constexpr Vector2& operator*=(T scalar) noexcept { x *= scalar; y *= scalar; return *this; }
        /// @brief Divides by another vector component-wise.
        constexpr Vector2& operator/=(Vector2 rhs) noexcept { x /= rhs.x; y /= rhs.y; return *this; }
        /// @brief Divides every component by a scalar.
        constexpr Vector2& operator/=(T scalar) noexcept { x /= scalar; y /= scalar; return *this; }

        /// @return The component-wise sum.
        [[nodiscard]] friend constexpr Vector2 operator+(Vector2 a, Vector2 b) noexcept { a += b; return a; }
        /// @return The component-wise difference.
        [[nodiscard]] friend constexpr Vector2 operator-(Vector2 a, Vector2 b) noexcept { a -= b; return a; }
        /// @return The component-wise product.
        [[nodiscard]] friend constexpr Vector2 operator*(Vector2 a, Vector2 b) noexcept { a *= b; return a; }
        /// @return The vector scaled by a scalar.
        [[nodiscard]] friend constexpr Vector2 operator*(Vector2 a, T scalar) noexcept { a *= scalar; return a; }
        /// @return The vector scaled by a scalar.
        [[nodiscard]] friend constexpr Vector2 operator*(T scalar, Vector2 rhs) noexcept { rhs *= scalar; return rhs; }
        /// @return The component-wise quotient.
        [[nodiscard]] friend constexpr Vector2 operator/(Vector2 a, Vector2 b) noexcept { a /= b; return a; }
        /// @return The vector divided by a scalar.
        [[nodiscard]] friend constexpr Vector2 operator/(Vector2 a, T scalar) noexcept { a /= scalar; return a; }
        /// @return A vector with the scalar divided by each component.
        [[nodiscard]] friend constexpr Vector2 operator/(T scalar, Vector2 b) noexcept {
            return Vector2(scalar / b.x, scalar / b.y);
        }

        /// @return The vector with every component negated.
        [[nodiscard]] constexpr Vector2 operator-() const noexcept { return Vector2(-x, -y); }

        /// @return The squared length, cheaper than Length.
        [[nodiscard]] constexpr T LengthSq() const noexcept { return x * x + y * y; }
        /// @return The length.
        [[nodiscard]] T Length() const noexcept requires std::floating_point<T> { return Operations::Sqrt<T>(LengthSq()); }

        /// @brief Scales the vector to length 1; a zero vector is left unchanged.
        void Normalize() noexcept requires std::floating_point<T> {
            T len = Length();
            if (len > T{0}) {
                x /= len;
                y /= len;
            }
        }

        /// @return A unit vector with the same direction, or a zero vector.
        [[nodiscard]] Vector2 Normalized() const noexcept requires std::floating_point<T> {
            T len = Length();
            if (len > T{0}) return Vector2(x / len, y / len);
            return Vector2(T{0});
        }

        /// @param rhs Other vector.
        /// @return The dot product.
        [[nodiscard]] constexpr T Dot(const Vector2& rhs) const noexcept  { return x * rhs.x + y * rhs.y; }
        /// @param rhs Other vector.
        /// @return The Z of the 3D cross product; positive when rhs is counter-clockwise from this vector.
        [[nodiscard]] constexpr T Cross(const Vector2& rhs) const noexcept { return x * rhs.y - y * rhs.x; }

        /// @param rhs Other point.
        /// @return The distance between the two points.
        [[nodiscard]] T Distance(const Vector2& rhs) const noexcept requires std::floating_point<T> { return (*this - rhs).Length(); }

        /// @param u Vector to project.
        /// @param v Direction to project onto.
        /// @return The part of u parallel to v, or zero when v is zero.
        [[nodiscard]] static constexpr Vector2 Project(const Vector2& u, const Vector2& v) noexcept requires std::floating_point<T> {
            T vLenSq = v.LengthSq();
            if (vLenSq <= T{0}) return Vector2(T{0});
            return v * (u.Dot(v) / vLenSq);
        }

        /// @param u Vector to split.
        /// @param v Reference direction.
        /// @return The part of u perpendicular to v.
        [[nodiscard]] static constexpr Vector2 Rejection(const Vector2& u, const Vector2& v) noexcept requires std::floating_point<T> {
            return u - Project(u, v);
        }

        /// @param start Value at t = 0.
        /// @param end Value at t = 1.
        /// @param t Interpolation factor; it is not clamped.
        /// @return The linear interpolation.
        [[nodiscard]] static constexpr Vector2 Lerp(const Vector2& start, const Vector2& end, T t) noexcept requires std::floating_point<T> {
            return start + (end - start) * t;
        }

        /// @param v Incoming vector.
        /// @param normal Unit surface normal.
        /// @return The vector mirrored by the surface.
        [[nodiscard]] static constexpr Vector2 Reflect(const Vector2& v, const Vector2& normal) noexcept requires std::floating_point<T> {
            return v - normal * (T{2} * v.Dot(normal));
        }

        /// @param a First vector.
        /// @param b Second vector.
        /// @param epsilon Tolerance per component.
        /// @return Whether every component differs by at most epsilon.
        [[nodiscard]] static constexpr bool NearlyEqual(const Vector2& a, const Vector2& b,
                                              T epsilon = Constants::Tolerance<T>) noexcept requires std::floating_point<T>{
            return Operations::NearlyEqual(a.x, b.x, epsilon) && Operations::NearlyEqual(a.y, b.y, epsilon);
        }

        /// @return A vector with every component 0.
        static constexpr Vector2 Zero() noexcept { return Vector2{ T{0}, T{0} }; }
        /// @return A vector with every component 1.
        static constexpr Vector2 One() noexcept { return Vector2{ T{1}, T{1} }; }
        /// @return The unit vector along +X.
        static constexpr Vector2 UnitX() noexcept {return  Vector2{ T{1}, T{0} }; }
        /// @return The unit vector along +Y.
        static constexpr Vector2 UnitY() noexcept { return  Vector2{ T{0}, T{1} }; }
        /// @return The unit vector along -X.
        static constexpr Vector2 NegUnitX() noexcept { return  Vector2{ -T{1}, T{0} }; }
        /// @return The unit vector along -Y.
        static constexpr Vector2 NegUnitY() noexcept { return  Vector2{ T{0}, -T{1} }; }
        /// @return A vector with every component +infinity.
        static constexpr Vector2 Infinity() noexcept { return  Vector2{Constants::Infinity<T>, Constants::Infinity<T> }; }
        /// @return A vector with every component -infinity.
        static constexpr Vector2 NegInfinity() noexcept { return  Vector2{Constants::NegInfinity<T>, Constants::NegInfinity<T> }; }
    };
    static_assert(sizeof(Vector2<float>) == 2 * sizeof(float));

    template<typename T>
    /// @brief 3-component vector.
    /// @tparam T Arithmetic type of the components.
    class Vector3 final {
    static_assert(std::is_arithmetic_v<T>, "Vector3 must be arithmetic.");
    public:
        /// X component.
        T x;
        /// Y component.
        T y;
        /// Z component.
        T z;

        /// @brief Creates a zero vector.
        constexpr Vector3() noexcept : x(T{}), y(T{}), z(T{}) {}
        /// @brief Creates a vector from its components.
        /// @param inX X component.
        /// @param inY Y component.
        /// @param inZ Z component.
        constexpr Vector3(T inX, T inY, T inZ) noexcept : x(inX), y(inY), z(inZ) {}
        /// @brief Creates a vector with every component equal.
        /// @param xyz Value of every component.
        explicit constexpr Vector3(T xyz) noexcept : x(xyz), y(xyz), z(xyz) {}

        /// @return A pointer to the contiguous components, for GPU uploads.
        [[nodiscard]] constexpr const T* Data() const noexcept { return &x; }
        /// @return A pointer to the contiguous components.
        [[nodiscard]] constexpr T* Data() noexcept { return &x; }

        /// @brief Adds another vector component-wise.
        constexpr Vector3& operator+=(Vector3 rhs) noexcept { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
        /// @brief Subtracts another vector component-wise.
        constexpr Vector3& operator-=(Vector3 rhs) noexcept { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
        /// @brief Multiplies by another vector component-wise.
        constexpr Vector3& operator*=(Vector3 rhs) noexcept { x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this; }
        /// @brief Multiplies every component by a scalar.
        constexpr Vector3& operator*=(T scalar) noexcept { x *= scalar; y *= scalar; z *= scalar; return *this; }
        /// @brief Divides by another vector component-wise.
        constexpr Vector3& operator/=(Vector3 rhs) noexcept { x /= rhs.x; y /= rhs.y; z /= rhs.z; return *this; }
        /// @brief Divides every component by a scalar.
        constexpr Vector3& operator/=(T scalar) noexcept { x /= scalar; y /= scalar; z /= scalar; return *this; }

        /// @return The component-wise sum.
        [[nodiscard]] friend constexpr Vector3 operator+(Vector3 a, Vector3 b) noexcept { a += b; return a; }
        /// @return The component-wise difference.
        [[nodiscard]] friend constexpr Vector3 operator-(Vector3 a, Vector3 b) noexcept { a -= b; return a; }
        /// @return The component-wise product.
        [[nodiscard]] friend constexpr Vector3 operator*(Vector3 a, Vector3 b) noexcept { a *= b; return a; }
        /// @return The vector scaled by a scalar.
        [[nodiscard]] friend constexpr Vector3 operator*(Vector3 a, T scalar) noexcept { a *= scalar; return a; }
        /// @return The vector scaled by a scalar.
        [[nodiscard]] friend constexpr Vector3 operator*(T scalar, Vector3 rhs) noexcept { rhs *= scalar; return rhs; }
        /// @return The component-wise quotient.
        [[nodiscard]] friend constexpr Vector3 operator/(Vector3 a, Vector3 b) noexcept { a /= b; return a; }
        /// @return The vector divided by a scalar.
        [[nodiscard]] friend constexpr Vector3 operator/(Vector3 a, T scalar) noexcept { a /= scalar; return a; }
        /// @return A vector with the scalar divided by each component.
        [[nodiscard]] friend constexpr Vector3 operator/(T scalar, Vector3 b) noexcept {
            return Vector3(scalar / b.x, scalar / b.y, scalar / b.z);
        }

        /// @return The vector with every component negated.
        [[nodiscard]] constexpr Vector3 operator-() const noexcept { return Vector3(-x, -y, -z); }

        /// @return The squared length, cheaper than Length.
        [[nodiscard]] constexpr T LengthSq() const noexcept { return x * x + y * y + z * z; }
        /// @return The length.
        [[nodiscard]] T Length() const noexcept requires std::floating_point<T> { return Operations::Sqrt<T>(LengthSq()); }

        /// @brief Scales the vector to length 1; a zero vector is left unchanged.
        void Normalize() noexcept requires std::floating_point<T> {
            T len = Length();
            if (len > T{0}) {
                x /= len;
                y /= len;
                z /= len;
            }
        }

        /// @return A unit vector with the same direction, or a zero vector.
        [[nodiscard]] Vector3 Normalized() const noexcept requires std::floating_point<T> {
            T len = Length();
            if (len > T{0}) return Vector3(x / len, y / len, z / len);
            return Vector3(T{0});
        }

        /// @param rhs Other vector.
        /// @return The dot product.
        [[nodiscard]] constexpr T Dot(const Vector3& rhs) const noexcept { return x * rhs.x + y * rhs.y +  z * rhs.z; }
        /// @param rhs Other vector.
        /// @return The right-handed cross product.
        [[nodiscard]] constexpr Vector3 Cross(const Vector3& rhs) const noexcept  {
            return Vector3{
                y * rhs.z - z * rhs.y,
                z * rhs.x - x * rhs.z,
                x * rhs.y - y * rhs.x
            };
        }

        /// @param rhs Other point.
        /// @return The distance between the two points.
        [[nodiscard]] T Distance(const Vector3& rhs) const noexcept requires std::floating_point<T> { return (*this - rhs).Length(); }

        /// @param u Vector to project.
        /// @param v Direction to project onto.
        /// @return The part of u parallel to v, or zero when v is zero.
        [[nodiscard]] static constexpr Vector3 Project(const Vector3& u, const Vector3& v) noexcept requires std::floating_point<T> {
            T vLenSq = v.LengthSq();
            if (vLenSq <= T{0}) return Vector3(T{0});
            return v * (u.Dot(v) / vLenSq);
        }

        /// @param u Vector to split.
        /// @param v Reference direction.
        /// @return The part of u perpendicular to v.
        [[nodiscard]] static constexpr Vector3 Rejection(const Vector3& u, const Vector3& v) noexcept requires std::floating_point<T> {
            return u - Project(u, v);
        }

        /// @param start Value at t = 0.
        /// @param end Value at t = 1.
        /// @param t Interpolation factor; it is not clamped.
        /// @return The linear interpolation.
        [[nodiscard]] static constexpr Vector3 Lerp(const Vector3& start, const Vector3& end, T t) noexcept requires std::floating_point<T> {
            return start + (end - start) * t;
        }

        /// @param v Incoming vector.
        /// @param normal Unit surface normal.
        /// @return The vector mirrored by the surface.
        [[nodiscard]] static constexpr Vector3 Reflect(const Vector3& v, const Vector3& normal) noexcept requires std::floating_point<T> {
            return v - normal * (T{2} * v.Dot(normal));
        }

        /// @param a First vector.
        /// @param b Second vector.
        /// @param epsilon Tolerance per component.
        /// @return Whether every component differs by at most epsilon.
        [[nodiscard]] static constexpr bool NearlyEqual(const Vector3& a, const Vector3& b,
                                              T epsilon = Constants::Tolerance<T>) noexcept requires std::floating_point<T> {
            return Operations::NearlyEqual(a.x, b.x, epsilon) && Operations::NearlyEqual(a.y, b.y, epsilon) &&
                   Operations::NearlyEqual(a.z, b.z, epsilon);
        }

        /// @return A vector with every component 0.
        [[nodiscard]] static constexpr Vector3 Zero() noexcept { return Vector3(T{0}, T{0}, T{0}); }
        /// @return A vector with every component 1.
        [[nodiscard]] static constexpr Vector3 One() noexcept { return Vector3(T{1}, T{1}, T{1}); }
        /// @return The unit vector along +X.
        [[nodiscard]] static constexpr Vector3 UnitX() noexcept { return Vector3(T{1}, T{0}, T{0}); }
        /// @return The unit vector along +Y.
        [[nodiscard]] static constexpr Vector3 UnitY() noexcept { return Vector3(T{0}, T{1}, T{0}); }
        /// @return The unit vector along +Z.
        [[nodiscard]] static constexpr Vector3 UnitZ() noexcept { return Vector3(T{0}, T{0}, T{1}); }
        /// @return The unit vector along -X.
        [[nodiscard]] static constexpr Vector3 NegUnitX() noexcept { return Vector3(T{-1}, T{0}, T{0}); }
        /// @return The unit vector along -Y.
        [[nodiscard]] static constexpr Vector3 NegUnitY() noexcept { return Vector3{T{0}, T{-1}, T{0}}; }
        /// @return The unit vector along -Z.
        [[nodiscard]] static constexpr Vector3 NegUnitZ() noexcept { return Vector3{T{0}, T{0}, T{-1}}; }
        /// @return A vector with every component +infinity.
        [[nodiscard]] static constexpr Vector3 Infinity() noexcept { return Vector3{Constants::Infinity<T>, Constants::Infinity<T>, Constants::Infinity<T>}; }
        /// @return A vector with every component -infinity.
        [[nodiscard]] static constexpr Vector3 NegInfinity() noexcept { return Vector3{Constants::NegInfinity<T>, Constants::NegInfinity<T>, Constants::NegInfinity<T>}; }
    };
    static_assert(sizeof(Vector3<float>) == 3 * sizeof(float));

    template<typename T>
    /// @brief 4-component vector.
    /// @tparam T Arithmetic type of the components.
    class Vector4 final {
    static_assert(std::is_arithmetic_v<T>, "Vector4 must be arithmetic.");
    public:
        /// X component.
        T x;
        /// Y component.
        T y;
        /// Z component.
        T z;
        /// W component.
        T w;

        /// @brief Creates a zero vector.
        constexpr Vector4() noexcept : x(T{}), y(T{}), z(T{}), w(T{}) {}
        /// @brief Creates a vector from its components.
        /// @param inX X component.
        /// @param inY Y component.
        /// @param inZ Z component.
        /// @param inW W component.
        constexpr Vector4(T inX, T inY, T inZ, T inW) noexcept : x(inX), y(inY), z(inZ), w(inW) {}
        /// @brief Creates a vector with every component equal.
        /// @param xyzw Value of every component.
        explicit constexpr Vector4(T xyzw) noexcept : x(xyzw), y(xyzw), z(xyzw), w(xyzw) {}
        /// @brief Creates a vector from a 3-component vector and a W value.
        /// @param vec3 X, Y and Z components.
        /// @param inW W component.
        constexpr Vector4(const Vector3<T>& vec3, T inW) noexcept
            : x(vec3.x), y(vec3.y), z(vec3.z), w(inW) {}

        /// @return A pointer to the contiguous components, for GPU uploads.
        [[nodiscard]] constexpr const T* Data() const noexcept { return &x; }
        /// @return A pointer to the contiguous components.
        [[nodiscard]] constexpr T* Data() noexcept { return &x; }

        /// @brief Adds another vector component-wise.
        constexpr Vector4& operator+=(Vector4 rhs) noexcept { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
        /// @brief Subtracts another vector component-wise.
        constexpr Vector4& operator-=(Vector4 rhs) noexcept { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
        /// @brief Multiplies by another vector component-wise.
        constexpr Vector4& operator*=(Vector4 rhs) noexcept { x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this; }
        /// @brief Multiplies every component by a scalar.
        constexpr Vector4& operator*=(T scalar) noexcept { x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this; }
        /// @brief Divides by another vector component-wise.
        constexpr Vector4& operator/=(Vector4 rhs) noexcept { x /= rhs.x; y /= rhs.y; z /= rhs.z; w /= rhs.w; return *this; }
        /// @brief Divides every component by a scalar.
        constexpr Vector4& operator/=(T scalar) noexcept { x /= scalar; y /= scalar; z /= scalar; w /= scalar; return *this; }

        /// @return The component-wise sum.
        [[nodiscard]] friend constexpr Vector4 operator+(Vector4 a, Vector4 b) noexcept { a += b; return a; }
        /// @return The component-wise difference.
        [[nodiscard]] friend constexpr Vector4 operator-(Vector4 a, Vector4 b) noexcept { a -= b; return a; }
        /// @return The component-wise product.
        [[nodiscard]] friend constexpr Vector4 operator*(Vector4 a, Vector4 b) noexcept { a *= b; return a; }
        /// @return The vector scaled by a scalar.
        [[nodiscard]] friend constexpr Vector4 operator*(Vector4 a, T scalar) noexcept { a *= scalar; return a; }
        /// @return The vector scaled by a scalar.
        [[nodiscard]] friend constexpr Vector4 operator*(T scalar, Vector4 rhs) noexcept { rhs *= scalar; return rhs; }
        /// @return The component-wise quotient.
        [[nodiscard]] friend constexpr Vector4 operator/(Vector4 a, Vector4 b) noexcept { a /= b; return a; }
        /// @return The vector divided by a scalar.
        [[nodiscard]] friend constexpr Vector4 operator/(Vector4 a, T scalar) noexcept { a /= scalar; return a; }
        /// @return A vector with the scalar divided by each component.
        [[nodiscard]] friend constexpr Vector4 operator/(T scalar, Vector4 b) noexcept {
            return Vector4(scalar / b.x, scalar / b.y, scalar / b.z, scalar / b.w);
        }

        /// @return The vector with every component negated.
        [[nodiscard]] constexpr Vector4 operator-() const noexcept { return Vector4(-x, -y, -z, -w); }

        /// @return The squared length, cheaper than Length.
        [[nodiscard]] constexpr T LengthSq() const noexcept { return x * x + y * y + z * z + w * w; }
        /// @return The length.
        [[nodiscard]] T Length() const noexcept requires std::floating_point<T> {
            return Operations::Sqrt<T>(LengthSq());
        }

        /// @brief Scales the vector to length 1; a zero vector is left unchanged.
        void Normalize() noexcept requires std::floating_point<T> {
            T len = Length();
            if (len > T{0}) {
                x /= len;
                y /= len;
                z /= len;
                w /= len;
            }
        }

        /// @return A unit vector with the same direction, or a zero vector.
        [[nodiscard]] Vector4 Normalized() const noexcept requires std::floating_point<T> {
            T len = Length();
            if (len > T{0}) return Vector4(x / len, y / len, z / len, w / len);
            return Vector4(T{0});
        }

        /// @param rhs Other vector.
        /// @return The dot product.
        [[nodiscard]] constexpr T Dot(const Vector4& rhs) const noexcept { return x * rhs.x + y * rhs.y +  z * rhs.z + w * rhs.w; }

        /// @param rhs Other point.
        /// @return The distance between the two points.
        [[nodiscard]] T Distance(const Vector4& rhs) const noexcept requires std::floating_point<T> { return (*this - rhs).Length(); }

        /// @param u Vector to project.
        /// @param v Direction to project onto.
        /// @return The part of u parallel to v, or zero when v is zero.
        [[nodiscard]] static constexpr Vector4 Project(const Vector4& u, const Vector4& v) noexcept requires std::floating_point<T> {
            T vLenSq = v.LengthSq();
            if (vLenSq <= T{0}) return Vector4(T{0});
            return v * (u.Dot(v) / vLenSq);
        }

        /// @param u Vector to split.
        /// @param v Reference direction.
        /// @return The part of u perpendicular to v.
        [[nodiscard]] static constexpr Vector4 Rejection(const Vector4& u, const Vector4& v) noexcept requires std::floating_point<T> {
            return u - Project(u, v);
        }

        /// @param start Value at t = 0.
        /// @param end Value at t = 1.
        /// @param t Interpolation factor; it is not clamped.
        /// @return The linear interpolation.
        [[nodiscard]] static constexpr Vector4 Lerp(const Vector4& start, const Vector4& end, T t) noexcept requires std::floating_point<T> {
            return start + (end - start) * t;
        }

        /// @param v Incoming vector.
        /// @param normal Unit surface normal.
        /// @return The vector mirrored by the surface.
        [[nodiscard]] static constexpr Vector4 Reflect(const Vector4& v, const Vector4& normal) noexcept requires std::floating_point<T> {
            return v - normal * (T{2} * v.Dot(normal));
        }

        /// @param a First vector.
        /// @param b Second vector.
        /// @param epsilon Tolerance per component.
        /// @return Whether every component differs by at most epsilon.
        [[nodiscard]] static constexpr  bool NearlyEqual(const Vector4& a, const Vector4& b,
                                              T epsilon = Constants::Tolerance<T>) noexcept requires std::floating_point<T>{
            return Operations::NearlyEqual(a.x, b.x, epsilon) && Operations::NearlyEqual(a.y, b.y, epsilon) &&
                   Operations::NearlyEqual(a.z, b.z, epsilon) && Operations::NearlyEqual(a.w, b.w, epsilon);
        }

        /// @return A vector with every component 0.
        static constexpr Vector4 Zero() noexcept { return Vector4{T{0}, T{0}, T{0}, T{0}}; }
        /// @return A vector with every component 1.
        static constexpr Vector4 One() noexcept { return  Vector4{T{1}, T{1}, T{1}, T{1}}; }
        /// @return (0, 0, 1, 1), the texture rectangle that covers a whole texture.
        static constexpr Vector4 UnitRect() noexcept { return Vector4{T{0}, T{0}, T{1}, T{1}}; }
        /// @return The unit vector along +X.
        static constexpr Vector4 UnitX() noexcept { return Vector4{T{1}, T{0}, T{0}, T{0}}; }
        /// @return The unit vector along +Y.
        static constexpr Vector4 UnitY() noexcept { return Vector4{T{0}, T{1}, T{0}, T{0}}; }
        /// @return The unit vector along +Z.
        static constexpr Vector4 UnitZ() noexcept { return Vector4{T{0}, T{0}, T{1}, T{0}}; }
        /// @return The unit vector along +W.
        static constexpr Vector4 UnitW() noexcept { return Vector4{T{0}, T{0}, T{0}, T{1}}; }
        /// @return The unit vector along -X.
        static constexpr Vector4 NegUnitX() noexcept { return Vector4{-T{1}, T{0}, T{0}, T{0}}; }
        /// @return The unit vector along -Y.
        static constexpr Vector4 NegUnitY() noexcept { return Vector4{T{0}, -T{1}, T{0}, T{0}}; }
        /// @return The unit vector along -Z.
        static constexpr Vector4 NegUnitZ() noexcept { return Vector4{T{0}, T{0}, -T{1}, T{0}}; }
        /// @return The unit vector along -W.
        static constexpr Vector4 NegUnitW() noexcept { return Vector4{T{0}, T{0}, T{0}, -T{1}}; }
        /// @return A vector with every component +infinity.
        static constexpr Vector4 Infinity() noexcept { return Vector4{Constants::Infinity<T>, Constants::Infinity<T>, Constants::Infinity<T>, Constants::Infinity<T>}; }
        /// @return A vector with every component -infinity.
        static constexpr Vector4 NegInfinity() noexcept { return Vector4{Constants::NegInfinity<T>, Constants::NegInfinity<T>,Constants::NegInfinity<T>,Constants::NegInfinity<T>}; }
    };
    static_assert(sizeof(Vector4<float>) == 4 * sizeof(float));

    /// 2D float vector.
    using Vector2f = Vector2<float>;
    /// 2D int vector.
    using Vector2i = Vector2<int>;

    /// 3D float vector.
    using Vector3f = Vector3<float>;
    /// 3D int vector.
    using Vector3i = Vector3<int>;

    /// 4D float vector, also used for RGBA colors.
    using Vector4f = Vector4<float>;
    /// 4D int vector.
    using Vector4i = Vector4<int>;
}