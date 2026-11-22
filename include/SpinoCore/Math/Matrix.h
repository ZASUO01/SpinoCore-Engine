// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <concepts>
#include <cstddef>
#include <type_traits>
#include "Math.h"
#include "Vector.h"

namespace SpinoCore::Math {
    template <typename T>
    class Matrix4;

    /// @brief 3x3 matrix stored in column-major order, as OpenGL expects.
    /// @tparam T Arithmetic type of the elements.
    template <typename T>
    class Matrix3 final {
        static_assert(std::is_arithmetic_v<T>, "Matrix3 must be of arithmetic type.");
    public:
        /// Elements in column-major order.
        T data[9];

        /// @brief Creates an identity matrix.
        constexpr Matrix3() noexcept :data{
            T{1}, T{0}, T{0},
            T{0}, T{1}, T{0},
            T{0}, T{0}, T{1}
        }{}

        /// @brief Creates a matrix from 9 elements in column-major order.
        /// @param inData Elements to copy.
        constexpr explicit Matrix3(const T inData[9]) noexcept {
            for (std::size_t i = 0; i < 9; ++i) {
                data[i] = inData[i];
            }
        }

        /// @brief Takes the upper-left 3x3 part of a 4x4 matrix.
        /// @param matrix Source matrix.
        constexpr explicit Matrix3(const Matrix4<T>& matrix) noexcept;

        /// @param row Row index.
        /// @param col Column index.
        /// @return The element at the row and column.
        [[nodiscard]] constexpr T& operator()(const std::size_t row, const std::size_t col) noexcept {
            return data[col * 3 + row];
        }

        /// @param row Row index.
        /// @param col Column index.
        /// @return The element at the row and column.
        [[nodiscard]] constexpr const T& operator()(const std::size_t row, const std::size_t col) const noexcept {
            return data[col * 3 + row];
        }

        /// @return A pointer to the elements, for GPU uploads.
        [[nodiscard]] constexpr const T* Data() const noexcept { return data; }
        /// @return A pointer to the elements.
        [[nodiscard]] constexpr T* Data() noexcept { return data; }

        /// @return The matrix product a times b; b is applied first to vectors.
        [[nodiscard]] friend constexpr Matrix3 operator*(const Matrix3& a, const Matrix3& b){
            Matrix3 retVal;

            for (std::size_t col = 0; col < 3; ++col) {
                for (std::size_t row = 0; row < 3; ++row) {
                    retVal(row, col) =
                        a(row, 0) * b(0, col) +
                        a(row, 1) * b(1, col) +
                        a(row, 2) * b(2, col);
                }
            }

            return retVal;
        }

        /// @brief Multiplies this matrix by another on the right.
        constexpr Matrix3& operator*=(const Matrix3& rhs) noexcept {
            *this = *this * rhs;
            return *this;
        }

        /// @param outSuccess Receives false when the matrix cannot be inverted; it may be nullptr.
        /// @return The inverse, or the identity when there is none.
        [[nodiscard]] constexpr Matrix3 GetInverse(bool* outSuccess = nullptr) const noexcept requires std::floating_point<T> {
            Matrix3 inv;

            const T cofactor00 = data[4] * data[8] - data[7] * data[5];
            const T cofactor10 = data[7] * data[2] - data[1] * data[8];
            const T cofactor20 = data[1] * data[5] - data[4] * data[2];

            const T cofactor01 = data[6] * data[5] - data[3] * data[8];
            const T cofactor11 = data[0] * data[8] - data[6] * data[2];
            const T cofactor21 = data[3] * data[2] - data[0] * data[5];

            const T cofactor02 = data[3] * data[7] - data[6] * data[4];
            const T cofactor12 = data[6] * data[1] - data[0] * data[7];
            const T cofactor22 = data[0] * data[4] - data[3] * data[1];

            const T det = data[0] * cofactor00 + data[3] * cofactor10 + data[6] * cofactor20;

            if (det == T{0}) {
                if (outSuccess) *outSuccess = false;
                return Matrix3();
            }

            const T invDet = T{1} / det;

            inv.data[0] = cofactor00 * invDet;
            inv.data[1] = cofactor10 * invDet;
            inv.data[2] = cofactor20 * invDet;

            inv.data[3] = cofactor01 * invDet;
            inv.data[4] = cofactor11 * invDet;
            inv.data[5] = cofactor21 * invDet;

            inv.data[6] = cofactor02 * invDet;
            inv.data[7] = cofactor12 * invDet;
            inv.data[8] = cofactor22 * invDet;

            if (outSuccess) *outSuccess = true;
            return inv;
        }

        /// @brief Inverts the matrix in place; a matrix without an inverse is left unchanged.
        constexpr void Invert() noexcept requires std::floating_point<T> {
            bool success = false;
            const Matrix3 inverse = GetInverse(&success);
            if (success) {
                *this = inverse;
            }
        }

        /// @return The matrix with rows and columns swapped.
        [[nodiscard]] constexpr Matrix3 GetTransposed() const noexcept {
            Matrix3 transposed;

            transposed.data[0] = data[0];
            transposed.data[4] = data[4];
            transposed.data[8] = data[8];

            transposed.data[1] = data[3]; transposed.data[3] = data[1];
            transposed.data[2] = data[6]; transposed.data[6] = data[2];
            transposed.data[5] = data[7]; transposed.data[7] = data[5];

            return transposed;
        }

        /// @brief Swaps rows and columns in place.
        constexpr void Transpose() noexcept {
            Operations::swap(data[1], data[3]);
            Operations::swap(data[2], data[6]);
            Operations::swap(data[5], data[7]);
        }

        /// @param v Direction.
        /// @return The direction transformed without translation.
        [[nodiscard]] constexpr Vector3<T> TransformVector(const Vector3<T>& v) const noexcept {
            return Vector3<T>(
                v.x * operator()(0, 0) + v.y * operator()(0, 1) + v.z * operator()(0, 2),
                v.x * operator()(1, 0) + v.y * operator()(1, 1) + v.z * operator()(1, 2),
                v.x * operator()(2, 0) + v.y * operator()(2, 1) + v.z * operator()(2, 2)
            );
        }

        /// @return The identity matrix.
        [[nodiscard]] static constexpr Matrix3 Identity() noexcept {
            return Matrix3();
        }
    };

    /// @brief 4x4 matrix stored in column-major order, as OpenGL expects.
    /// @tparam T Arithmetic type of the elements.
    template <typename T>
    class Matrix4 final {
        static_assert(std::is_arithmetic_v<T>, "Matrix4 must be of arithmetic type.");
    public:
        /// Elements in column-major order.
        T data[16];

        /// @brief Creates an identity matrix.
        constexpr Matrix4() noexcept :data{
            T{1}, T{0}, T{0}, T{0},
            T{0}, T{1}, T{0}, T{0},
            T{0}, T{0}, T{1}, T{0},
            T{0}, T{0}, T{0}, T{1},
        }{}

        /// @brief Creates a matrix from 16 elements in column-major order.
        /// @param inData Elements to copy.
        constexpr explicit Matrix4(const T inData[16]) noexcept {
            for (std::size_t i = 0; i < 16; ++i) {
                data[i] = inData[i];
            }
        }

        /// @param row Row index.
        /// @param col Column index.
        /// @return The element at the row and column.
        [[nodiscard]] constexpr T& operator()(const std::size_t row, const std::size_t col) noexcept {
            return data[col * 4 + row];
        }

        /// @param row Row index.
        /// @param col Column index.
        /// @return The element at the row and column.
        [[nodiscard]] constexpr const T& operator()(const std::size_t row, const std::size_t col) const noexcept {
            return data[col * 4 + row];
        }

        /// @return A pointer to the elements, for GPU uploads.
        [[nodiscard]] constexpr const T* Data() const noexcept { return data; }
        /// @return A pointer to the elements.
        [[nodiscard]] constexpr T* Data() noexcept { return data; }

        /// @return The matrix product a times b; b is applied first to vectors.
        [[nodiscard]] friend constexpr Matrix4 operator*(const Matrix4& a, const Matrix4& b){
		    Matrix4 retVal;

            for (std::size_t col = 0; col < 4; ++col) {
                for (std::size_t row = 0; row < 4; ++row) {
                    retVal(row, col) =
                        a(row, 0) * b(0, col) +
                        a(row, 1) * b(1, col) +
                        a(row, 2) * b(2, col) +
                        a(row, 3) * b(3, col);
                }
            }

		    return retVal;
	    }

        /// @brief Multiplies this matrix by another on the right.
        constexpr Matrix4& operator*=(const Matrix4& rhs) noexcept {
            *this = *this * rhs;
            return *this;
        }

        /// @param outSuccess Receives false when the matrix cannot be inverted; it may be nullptr.
        /// @return The inverse, or the identity when there is none.
        [[nodiscard]] constexpr Matrix4 GetInverse(bool* outSuccess = nullptr) const noexcept requires std::floating_point<T> {
            Matrix4 inv;

            const T subFactor00 = data[10] * data[15] - data[14] * data[11];
            const T subFactor01 = data[6]  * data[15] - data[14] * data[7];
            const T subFactor02 = data[6]  * data[11] - data[10] * data[7];
            const T subFactor03 = data[2]  * data[15] - data[14] * data[3];
            const T subFactor04 = data[2]  * data[11] - data[10] * data[3];
            const T subFactor05 = data[2]  * data[7]  - data[6]  * data[3];
            const T subFactor06 = data[8]  * data[13] - data[12] * data[9];
            const T subFactor07 = data[4]  * data[13] - data[12] * data[5];
            const T subFactor08 = data[4]  * data[9]  - data[8]  * data[5];
            const T subFactor09 = data[0]  * data[13] - data[12] * data[1];
            const T subFactor10 = data[0]  * data[9]  - data[8]  * data[1];
            const T subFactor11 = data[0]  * data[5]  - data[4]  * data[1];

            inv.data[0]  =  data[5] * subFactor00 - data[9] * subFactor01 + data[13] * subFactor02;
            inv.data[1]  = -(data[1] * subFactor00 - data[9] * subFactor03 + data[13] * subFactor04);
            inv.data[2]  =  data[1] * subFactor01 - data[5] * subFactor03 + data[13] * subFactor05;
            inv.data[3]  = -(data[1] * subFactor02 - data[5] * subFactor04 + data[9]  * subFactor05);

            inv.data[4]  = -(data[4] * subFactor00 - data[8] * subFactor01 + data[12] * subFactor02);
            inv.data[5]  =  data[0] * subFactor00 - data[8] * subFactor03 + data[12] * subFactor04;
            inv.data[6]  = -(data[0] * subFactor01 - data[4] * subFactor03 + data[12] * subFactor05);
            inv.data[7]  =  data[0] * subFactor02 - data[4] * subFactor04 + data[8]  * subFactor05;

            inv.data[8]  =  data[7] * subFactor06 - data[11] * subFactor07 + data[15] * subFactor08;
            inv.data[9]  = -(data[3] * subFactor06 - data[11] * subFactor09 + data[15] * subFactor10);
            inv.data[10] =  data[3] * subFactor07 - data[7] * subFactor09 + data[15] * subFactor11;
            inv.data[11] = -(data[3] * subFactor08 - data[7] * subFactor10 + data[11] * subFactor11);

            inv.data[12] = -(data[6] * subFactor06 - data[10] * subFactor07 + data[14] * subFactor08);
            inv.data[13] =  data[2] * subFactor06 - data[10] * subFactor09 + data[14] * subFactor10;
            inv.data[14] = -(data[2] * subFactor07 - data[6] * subFactor09 + data[14] * subFactor11);
            inv.data[15] =  data[2] * subFactor08 - data[6] * subFactor10 + data[10] * subFactor11;

            const T det = data[0] * inv.data[0] + data[1] * inv.data[4] + data[2] * inv.data[8] + data[3] * inv.data[12];

            if (det == T{0}) {
                if (outSuccess) *outSuccess = false;
                return Matrix4();
            }

            const T invDet = T{1} / det;

            for (auto & i : inv.data) {
                i *= invDet;
            }

            if (outSuccess) *outSuccess = true;
            return inv;
        }

        /// @brief Inverts the matrix in place; a matrix without an inverse is left unchanged.
        constexpr void Invert() noexcept requires std::floating_point<T> {
            bool success = false;
            const Matrix4 inverse = GetInverse(&success);
            if (success) {
                *this = inverse;
            }
        }

        /// @return The matrix with rows and columns swapped.
        [[nodiscard]] constexpr Matrix4 GetTransposed() const noexcept {
            Matrix4 transposed;

            transposed.data[0] = data[0];
            transposed.data[5] = data[5];
            transposed.data[10] = data[10];
            transposed.data[15] = data[15];

            transposed.data[1] = data[4];  transposed.data[4] = data[1];
            transposed.data[2] = data[8];  transposed.data[8] = data[2];
            transposed.data[3] = data[12]; transposed.data[12] = data[3];
            transposed.data[6] = data[9];  transposed.data[9] = data[6];
            transposed.data[7] = data[13]; transposed.data[13] = data[7];
            transposed.data[11] = data[14]; transposed.data[14] = data[11];

            return transposed;
        }

        /// @brief Swaps rows and columns in place.
        constexpr void Transpose() noexcept {
            Operations::swap(data[1], data[4]);
            Operations::swap(data[2], data[8]);
            Operations::swap(data[3], data[12]);
            Operations::swap(data[6], data[9]);
            Operations::swap(data[7], data[13]);
            Operations::swap(data[11], data[14]);
        }

        /// @param v Direction.
        /// @return The direction transformed without translation.
        [[nodiscard]] constexpr Vector3<T> TransformVector(const Vector3<T>& v) const noexcept {
            return Vector3<T>(
                v.x * operator()(0, 0) + v.y * operator()(0, 1) + v.z * operator()(0, 2),
                v.x * operator()(1, 0) + v.y * operator()(1, 1) + v.z * operator()(1, 2),
                v.x * operator()(2, 0) + v.y * operator()(2, 1) + v.z * operator()(2, 2)
            );
        }

        /// @param v Point.
        /// @return The point transformed with translation and divided by w.
        [[nodiscard]] constexpr Vector3<T> TransformPoint(const Vector3<T>& v) const noexcept requires std::floating_point<T> {
            T x = v.x * operator()(0, 0) + v.y * operator()(0, 1) + v.z * operator()(0, 2) + operator()(0, 3);
            T y = v.x * operator()(1, 0) + v.y * operator()(1, 1) + v.z * operator()(1, 2) + operator()(1, 3);
            T z = v.x * operator()(2, 0) + v.y * operator()(2, 1) + v.z * operator()(2, 2) + operator()(2, 3);
            T w = v.x * operator()(3, 0) + v.y * operator()(3, 1) + v.z * operator()(3, 2) + operator()(3, 3);

            if (w != T{1} && !Operations::NearlyZero(w)) {
                const T invW = T{1} / w;
                x *= invW; y *= invW; z *= invW;
            }

            return Vector3<T>{x, y, z};
        }

        /// @return The matrix times a 4-component vector.
        [[nodiscard]] friend constexpr Vector4<T> operator*(const Matrix4& m, const Vector4<T>& v) noexcept {
            return Vector4<T>(
                v.x * m(0, 0) + v.y * m(0, 1) + v.z * m(0, 2) + v.w * m(0, 3),
                v.x * m(1, 0) + v.y * m(1, 1) + v.z * m(1, 2) + v.w * m(1, 3),
                v.x * m(2, 0) + v.y * m(2, 1) + v.z * m(2, 2) + v.w * m(2, 3),
                v.x * m(3, 0) + v.y * m(3, 1) + v.z * m(3, 2) + v.w * m(3, 3)
            );
        }

        /// @return The identity matrix.
        [[nodiscard]] static constexpr Matrix4 Identity() noexcept {
            return Matrix4();
        }
    };

    template <typename T>
    constexpr Matrix3<T>::Matrix3(const Matrix4<T>& matrix) noexcept {
        for (std::size_t col = 0; col < 3; ++col) {
            for (std::size_t row = 0; row < 3; ++row) {
                data[col * 3 + row] = matrix(row, col);
            }
        }
    }

    /// 3x3 float matrix.
    using Matrix3f = Matrix3<float>;
    /// 4x4 float matrix.
    using Matrix4f = Matrix4<float>;
}