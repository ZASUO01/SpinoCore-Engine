// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <type_traits>
#include "SpinoCore/Math/Math.h"

namespace SpinoCore::Math {
    template<typename T>
    class Vector2 final {
        static_assert(std::is_arithmetic_v<T>, "Vector2 must be arithmetic.");
    public:
        T x;
        T y;

        constexpr Vector2() noexcept :x({}), y({}) {}
        constexpr Vector2(T inX, T inY) noexcept : x(inX), y(inY) {}
        explicit constexpr Vector2(T xy) noexcept : x(xy), y(xy) {}

        [[nodiscard]] constexpr const T* Data() const noexcept { return &x; }
        [[nodiscard]] constexpr T* Data() noexcept { return &x; }

        constexpr Vector2& operator+=(Vector2 rhs) noexcept { x += rhs.x; y += rhs.y; return *this; }
        constexpr Vector2& operator-=(Vector2 rhs) noexcept { x -= rhs.x; y -= rhs.y; return *this; }
        constexpr Vector2& operator*=(Vector2 rhs) noexcept { x *= rhs.x; y *= rhs.y; return *this; }
        constexpr Vector2& operator*=(T scalar) noexcept { x *= scalar; y *= scalar; return *this; }
        constexpr Vector2& operator/=(Vector2 rhs) noexcept { x /= rhs.x; y /= rhs.y; return *this; }
        constexpr Vector2& operator/=(T scalar) noexcept { x /= scalar; y /= scalar; return *this; }

        [[nodiscard]] friend constexpr Vector2 operator+(Vector2 a, Vector2 b) noexcept { a += b; return a; }
        [[nodiscard]] friend constexpr Vector2 operator-(Vector2 a, Vector2 b) noexcept { a -= b; return a; }
        [[nodiscard]] friend constexpr Vector2 operator*(Vector2 a, Vector2 b) noexcept { a *= b; return a; }
        [[nodiscard]] friend constexpr Vector2 operator*(Vector2 a, T scalar) noexcept { a *= scalar; return a; }
        [[nodiscard]] friend constexpr Vector2 operator*(T scalar, Vector2 rhs) noexcept { rhs *= scalar; return rhs; }
        [[nodiscard]] friend constexpr Vector2 operator/(Vector2 a, Vector2 b) noexcept { a /= b; return a; }
        [[nodiscard]] friend constexpr Vector2 operator/(Vector2 a, T scalar) noexcept { a /= scalar; return a; }
        [[nodiscard]] friend constexpr Vector2 operator/(T scalar, Vector2 b) noexcept {
            return Vector2(scalar / b.x, scalar / b.y);
        }

        [[nodiscard]] constexpr Vector2 operator-() const noexcept { return Vector2(-x, -y); }

        [[nodiscard]] constexpr T LengthSq() const noexcept { return x * x + y * y; }
        [[nodiscard]] T Length() const noexcept requires std::floating_point<T> { return Operations::Sqrt<T>(LengthSq()); }

        void Normalize() noexcept requires std::floating_point<T> {
            T len = Length();
            if (len > T{0}) {
                x /= len;
                y /= len;
            }
        }

        [[nodiscard]] Vector2 Normalized() const noexcept requires std::floating_point<T> {
            T len = Length();
            if (len > T{0}) return Vector2(x / len, y / len);
            return Vector2(T{0});
        }

        [[nodiscard]] constexpr T Dot(const Vector2& rhs) const noexcept  { return x * rhs.x + y * rhs.y; }
        [[nodiscard]] constexpr T Cross(const Vector2& rhs) const noexcept { return x * rhs.y - y * rhs.x; }

        [[nodiscard]] T Distance(const Vector2& rhs) const noexcept requires std::floating_point<T> { return (*this - rhs).Length(); }

        [[nodiscard]] static constexpr Vector2 Project(const Vector2& u, const Vector2& v) noexcept requires std::floating_point<T> {
            T vLenSq = v.LengthSq();
            if (vLenSq <= T{0}) return Vector2(T{0});
            return v * (u.Dot(v) / vLenSq);
        }

        [[nodiscard]] static constexpr Vector2 Rejection(const Vector2& u, const Vector2& v) noexcept requires std::floating_point<T> {
            return u - Project(u, v);
        }

        [[nodiscard]] static constexpr Vector2 Lerp(const Vector2& start, const Vector2& end, T t) noexcept requires std::floating_point<T> {
            return start + (end - start) * t;
        }

        [[nodiscard]] static constexpr Vector2 Reflect(const Vector2& v, const Vector2& normal) noexcept requires std::floating_point<T> {
            return v - normal * (T{2} * v.Dot(normal));
        }

        [[nodiscard]] static constexpr bool NearlyEqual(const Vector2& a, const Vector2& b,
                                              T epsilon = Constants::Tolerance<T>) noexcept requires std::floating_point<T>{
            return Operations::NearlyEqual(a.x, b.x, epsilon) && Operations::NearlyEqual(a.y, b.y, epsilon);
        }

        static constexpr Vector2 Zero() noexcept { return Vector2{ T{0}, T{0} }; }
        static constexpr Vector2 One() noexcept { return Vector2{ T{1}, T{1} }; }
        static constexpr Vector2 UnitX() noexcept {return  Vector2{ T{1}, T{0} }; }
        static constexpr Vector2 UnitY() noexcept { return  Vector2{ T{0}, T{1} }; }
        static constexpr Vector2 NegUnitX() noexcept { return  Vector2{ -T{1}, T{0} }; }
        static constexpr Vector2 NegUnitY() noexcept { return  Vector2{ T{0}, -T{1} }; }
        static constexpr Vector2 Infinity() noexcept { return  Vector2{Constants::Infinity<T>, Constants::Infinity<T> }; }
        static constexpr Vector2 NegInfinity() noexcept { return  Vector2{Constants::NegInfinity<T>, Constants::NegInfinity<T> }; }
    };
    static_assert(sizeof(Vector2<float>) == 2 * sizeof(float));

    template<typename T>
    class Vector3 final {
    static_assert(std::is_arithmetic_v<T>, "Vector3 must be arithmetic.");
    public:
        T x;
        T y;
        T z;

        constexpr Vector3() noexcept : x(T{}), y(T{}), z(T{}) {}
        constexpr Vector3(T inX, T inY, T inZ) noexcept : x(inX), y(inY), z(inZ) {}
        explicit constexpr Vector3(T xyz) noexcept : x(xyz), y(xyz), z(xyz) {}

        [[nodiscard]] constexpr const T* Data() const noexcept { return &x; }
        [[nodiscard]] constexpr T* Data() noexcept { return &x; }

        constexpr Vector3& operator+=(Vector3 rhs) noexcept { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
        constexpr Vector3& operator-=(Vector3 rhs) noexcept { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
        constexpr Vector3& operator*=(Vector3 rhs) noexcept { x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this; }
        constexpr Vector3& operator*=(T scalar) noexcept { x *= scalar; y *= scalar; z *= scalar; return *this; }
        constexpr Vector3& operator/=(Vector3 rhs) noexcept { x /= rhs.x; y /= rhs.y; z /= rhs.z; return *this; }
        constexpr Vector3& operator/=(T scalar) noexcept { x /= scalar; y /= scalar; z /= scalar; return *this; }

        [[nodiscard]] friend constexpr Vector3 operator+(Vector3 a, Vector3 b) noexcept { a += b; return a; }
        [[nodiscard]] friend constexpr Vector3 operator-(Vector3 a, Vector3 b) noexcept { a -= b; return a; }
        [[nodiscard]] friend constexpr Vector3 operator*(Vector3 a, Vector3 b) noexcept { a *= b; return a; }
        [[nodiscard]] friend constexpr Vector3 operator*(Vector3 a, T scalar) noexcept { a *= scalar; return a; }
        [[nodiscard]] friend constexpr Vector3 operator*(T scalar, Vector3 rhs) noexcept { rhs *= scalar; return rhs; }
        [[nodiscard]] friend constexpr Vector3 operator/(Vector3 a, Vector3 b) noexcept { a /= b; return a; }
        [[nodiscard]] friend constexpr Vector3 operator/(Vector3 a, T scalar) noexcept { a /= scalar; return a; }
        [[nodiscard]] friend constexpr Vector3 operator/(T scalar, Vector3 b) noexcept {
            return Vector3(scalar / b.x, scalar / b.y, scalar / b.z);
        }

        [[nodiscard]] constexpr Vector3 operator-() const noexcept { return Vector3(-x, -y, -z); }

        [[nodiscard]] constexpr T LengthSq() const noexcept { return x * x + y * y + z * z; }
        [[nodiscard]] T Length() const noexcept requires std::floating_point<T> { return Operations::Sqrt<T>(LengthSq()); }

        void Normalize() noexcept requires std::floating_point<T> {
            T len = Length();
            if (len > T{0}) {
                x /= len;
                y /= len;
                z /= len;
            }
        }

        [[nodiscard]] Vector3 Normalized() const noexcept requires std::floating_point<T> {
            T len = Length();
            if (len > T{0}) return Vector3(x / len, y / len, z / len);
            return Vector3(T{0});
        }

        [[nodiscard]] constexpr T Dot(const Vector3& rhs) const noexcept { return x * rhs.x + y * rhs.y +  z * rhs.z; }
        [[nodiscard]] constexpr Vector3 Cross(const Vector3& rhs) const noexcept  {
            return Vector3{
                y * rhs.z - z * rhs.y,
                z * rhs.x - x * rhs.z,
                x * rhs.y - y * rhs.x
            };
        }

        [[nodiscard]] T Distance(const Vector3& rhs) const noexcept requires std::floating_point<T> { return (*this - rhs).Length(); }

        [[nodiscard]] static constexpr Vector3 Project(const Vector3& u, const Vector3& v) noexcept requires std::floating_point<T> {
            T vLenSq = v.LengthSq();
            if (vLenSq <= T{0}) return Vector3(T{0});
            return v * (u.Dot(v) / vLenSq);
        }

        [[nodiscard]] static constexpr Vector3 Rejection(const Vector3& u, const Vector3& v) noexcept requires std::floating_point<T> {
            return u - Project(u, v);
        }

        [[nodiscard]] static constexpr Vector3 Lerp(const Vector3& start, const Vector3& end, T t) noexcept requires std::floating_point<T> {
            return start + (end - start) * t;
        }

        [[nodiscard]] static constexpr Vector3 Reflect(const Vector3& v, const Vector3& normal) noexcept requires std::floating_point<T> {
            return v - normal * (T{2} * v.Dot(normal));
        }

        [[nodiscard]] static constexpr bool NearlyEqual(const Vector3& a, const Vector3& b,
                                              T epsilon = Constants::Tolerance<T>) noexcept requires std::floating_point<T> {
            return Operations::NearlyEqual(a.x, b.x, epsilon) && Operations::NearlyEqual(a.y, b.y, epsilon) &&
                   Operations::NearlyEqual(a.z, b.z, epsilon);
        }

        [[nodiscard]] static constexpr Vector3 Zero() noexcept { return Vector3(T{0}, T{0}, T{0}); }
        [[nodiscard]] static constexpr Vector3 One() noexcept { return Vector3(T{1}, T{1}, T{1}); }
        [[nodiscard]] static constexpr Vector3 UnitX() noexcept { return Vector3(T{1}, T{0}, T{0}); }
        [[nodiscard]] static constexpr Vector3 UnitY() noexcept { return Vector3(T{0}, T{1}, T{0}); }
        [[nodiscard]] static constexpr Vector3 UnitZ() noexcept { return Vector3(T{0}, T{0}, T{1}); }
        [[nodiscard]] static constexpr Vector3 NegUnitX() noexcept { return Vector3(T{-1}, T{0}, T{0}); }
        [[nodiscard]] static constexpr Vector3 NegUnitY() noexcept { return Vector3{T{0}, T{-1}, T{0}}; }
        [[nodiscard]] static constexpr Vector3 NegUnitZ() noexcept { return Vector3{T{0}, T{0}, T{-1}}; }
        [[nodiscard]] static constexpr Vector3 Infinity() noexcept { return Vector3{Constants::Infinity<T>, Constants::Infinity<T>, Constants::Infinity<T>}; }
        [[nodiscard]] static constexpr Vector3 NegInfinity() noexcept { return Vector3{Constants::NegInfinity<T>, Constants::NegInfinity<T>, Constants::NegInfinity<T>}; }
    };
    static_assert(sizeof(Vector3<float>) == 3 * sizeof(float));

    template<typename T>
    class Vector4 final {
    static_assert(std::is_arithmetic_v<T>, "Vector4 must be arithmetic.");
    public:
        T x;
        T y;
        T z;
        T w;

        constexpr Vector4() noexcept :x({}), y({}), z({}), w({}) {}
        constexpr Vector4(T inX, T inY, T inZ, T inW) noexcept : x(inX), y(inY), z(inZ), w(inW) {}
        explicit constexpr Vector4(T xyzw) noexcept : x(xyzw), y(xyzw), z(xyzw), w(xyzw) {}
        constexpr Vector4(const Vector3<T>& vec3, T inW) noexcept
            : x(vec3.x), y(vec3.y), z(vec3.z), w(inW) {}

        [[nodiscard]] constexpr const T* Data() const noexcept { return &x; }
        [[nodiscard]] constexpr T* Data() noexcept { return &x; }

        constexpr Vector4& operator+=(Vector4 rhs) noexcept { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
        constexpr Vector4& operator-=(Vector4 rhs) noexcept { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
        constexpr Vector4& operator*=(Vector4 rhs) noexcept { x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this; }
        constexpr Vector4& operator*=(T scalar) noexcept { x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this; }
        constexpr Vector4& operator/=(Vector4 rhs) noexcept { x /= rhs.x; y /= rhs.y; z /= rhs.z; w /= rhs.w; return *this; }
        constexpr Vector4& operator/=(T scalar) noexcept { x /= scalar; y /= scalar; z /= scalar; w /= scalar; return *this; }

        [[nodiscard]] friend constexpr Vector4 operator+(Vector4 a, Vector4 b) noexcept { a += b; return a; }
        [[nodiscard]] friend constexpr Vector4 operator-(Vector4 a, Vector4 b) noexcept { a -= b; return a; }
        [[nodiscard]] friend constexpr Vector4 operator*(Vector4 a, Vector4 b) noexcept { a *= b; return a; }
        [[nodiscard]] friend constexpr Vector4 operator*(Vector4 a, T scalar) noexcept { a *= scalar; return a; }
        [[nodiscard]] friend constexpr Vector4 operator*(T scalar, Vector4 rhs) noexcept { rhs *= scalar; return rhs; }
        [[nodiscard]] friend constexpr Vector4 operator/(Vector4 a, Vector4 b) noexcept { a /= b; return a; }
        [[nodiscard]] friend constexpr Vector4 operator/(Vector4 a, T scalar) noexcept { a /= scalar; return a; }
        [[nodiscard]] friend constexpr Vector4 operator/(T scalar, Vector4 b) noexcept {
            return Vector4(scalar / b.x, scalar / b.y, scalar / b.z, scalar / b.w);
        }

        [[nodiscard]] constexpr Vector4 operator-() const noexcept { return Vector4(-x, -y, -z, -w); }

        [[nodiscard]] constexpr T LengthSq() const noexcept { return x * x + y * y + z * z + w * w; }
        [[nodiscard]] T Length() const noexcept requires std::floating_point<T> {
            return Operations::Sqrt<T>(LengthSq());
        }

        void Normalize() noexcept requires std::floating_point<T> {
            T len = Length();
            if (len > T{0}) {
                x /= len;
                y /= len;
                z /= len;
                w /= len;
            }
        }

        [[nodiscard]] Vector4 Normalized() const noexcept requires std::floating_point<T> {
            T len = Length();
            if (len > T{0}) return Vector4(x / len, y / len, z / len, w / len);
            return Vector4(T{0});
        }

        [[nodiscard]] constexpr T Dot(const Vector4& rhs) const noexcept { return x * rhs.x + y * rhs.y +  z * rhs.z + w * rhs.w; }

        [[nodiscard]] T Distance(const Vector4& rhs) const noexcept requires std::floating_point<T> { return (*this - rhs).Length(); }

        [[nodiscard]] static constexpr Vector4 Project(const Vector4& u, const Vector4& v) noexcept requires std::floating_point<T> {
            T vLenSq = v.LengthSq();
            if (vLenSq <= T{0}) return Vector4(T{0});
            return v * (u.Dot(v) / vLenSq);
        }

        [[nodiscard]] static constexpr Vector4 Rejection(const Vector4& u, const Vector4& v) noexcept requires std::floating_point<T> {
            return u - Project(u, v);
        }

        [[nodiscard]] static constexpr Vector4 Lerp(const Vector4& start, const Vector4& end, T t) noexcept requires std::floating_point<T> {
            return start + (end - start) * t;
        }

        [[nodiscard]] static constexpr Vector4 Reflect(const Vector4& v, const Vector4& normal) noexcept requires std::floating_point<T> {
            return v - normal * (T{2} * v.Dot(normal));
        }

        [[nodiscard]] static constexpr  bool NearlyEqual(const Vector4& a, const Vector4& b,
                                              T epsilon = Constants::Tolerance<T>) noexcept requires std::floating_point<T>{
            return Operations::NearlyEqual(a.x, b.x, epsilon) && Operations::NearlyEqual(a.y, b.y, epsilon) &&
                   Operations::NearlyEqual(a.z, b.z, epsilon) && Operations::NearlyEqual(a.w, b.w, epsilon);
        }

        static constexpr Vector4 Zero() noexcept { return Vector4{T{0}, T{0}, T{0}, T{0}}; }
        static constexpr Vector4 One() noexcept { return  Vector4{T{1}, T{1}, T{1}, T{1}}; }
        static constexpr Vector4 UnitRect() noexcept { return Vector4{T{0}, T{0}, T{1}, T{1}}; }
        static constexpr Vector4 UnitX() noexcept { return Vector4{T{1}, T{0}, T{0}, T{0}}; }
        static constexpr Vector4 UnitY() noexcept { return Vector4{T{0}, T{1}, T{0}, T{0}}; }
        static constexpr Vector4 UnitZ() noexcept { return Vector4{T{0}, T{0}, T{1}, T{0}}; }
        static constexpr Vector4 UnitW() noexcept { return Vector4{T{0}, T{0}, T{0}, T{1}}; }
        static constexpr Vector4 NegUnitX() noexcept { return Vector4{-T{1}, T{0}, T{0}, T{0}}; }
        static constexpr Vector4 NegUnitY() noexcept { return Vector4{T{0}, -T{1}, T{0}, T{0}}; }
        static constexpr Vector4 NegUnitZ() noexcept { return Vector4{T{0}, T{0}, -T{1}, T{0}}; }
        static constexpr Vector4 NegUnitW() noexcept { return Vector4{T{0}, T{0}, T{0}, -T{1}}; }
        static constexpr Vector4 Infinity() noexcept { return Vector4{Constants::Infinity<T>, Constants::Infinity<T>, Constants::Infinity<T>, Constants::Infinity<T>}; }
        static constexpr Vector4 NegInfinity() noexcept { return Vector4{Constants::NegInfinity<T>, Constants::NegInfinity<T>,Constants::NegInfinity<T>,Constants::NegInfinity<T>}; }
    };
    static_assert(sizeof(Vector4<float>) == 4 * sizeof(float));

    using Vector2f = Vector2<float>;
    using Vector2i = Vector2<int>;

    using Vector3f = Vector3<float>;
    using Vector3i = Vector3<int>;

    using Vector4f = Vector4<float>;
    using Vector4i = Vector4<int>;
}