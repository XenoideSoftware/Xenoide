
#pragma once

#ifndef __XE_MATH_QUATERNION_HPP__
#define __XE_MATH_QUATERNION_HPP__

#include <cassert>
#include <cmath>
#include <cstdint>

#include "Common.h"
#include "Rotation.h"
#include "Vector.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4201) // non-standard extension used: nameless struct/union
#endif

namespace xe {
    template <typename T> struct TQuaternion {
        union {
            struct {
                TVector<T, 3> V;
                T W;
            };

            T values[4];
        };

        TQuaternion() {
            V.x = V.y = V.z = W = T(0);
        }

        explicit TQuaternion(const T w) {
            V.x = V.y = V.z = T(0);
            W = w;
        }

        explicit TQuaternion(const T *values) {
            assert(values);

            for (int i = 0; i < 4; i++) {
                this->values[i] = values[i];
            }
        }

        TQuaternion(const T x, const T y, const T z, const T w) {
            V.x = x;
            V.y = y;
            V.z = z;
            W = w;
        }

        explicit TQuaternion(const TVector<T, 3> &v) {
            V = v;
            W = T(0);
        }

        explicit TQuaternion(const T x, const T y, const T z) {
            V.x = x;
            V.y = y;
            V.z = z;
            W = T(0);
        }

        TQuaternion(const TVector<T, 3> &v, T w) {
            V = v;
            W = w;
        }

        explicit TQuaternion(const TVector<T, 4> &v) {
            V.x = v.x;
            V.y = v.y;
            V.z = v.z;
            W = v.w;
        }

        TQuaternion(const TQuaternion<T> &other) {
            V = other.V;
            W = other.W;
        }

        TQuaternion<T> &operator=(const TQuaternion<T> &other) {
            V = other.V;
            W = other.W;

            return *this;
        }

        [[nodiscard]] constexpr size_t size() const {
            return 4;
        }

        T *data() {
            return &values[0];
        }

        [[nodiscard]] const T *data() const {
            return &values[0];
        }

        constexpr const T &operator[](const size_t index) const {
            assert(index < 4);

            return values[index];
        }

        constexpr T &operator[](const size_t index) {
            assert(index < 4);

            return values[index];
        }

        explicit operator Rotation<T>() const {
            const T angle = T(2) * std::acos(W);

            if (angle == T(0)) {
                return {angle, {T(1), T(0), T(0)}};
            } else {
                return {angle, normalize(V)};
            }
        }

        explicit operator TVector<T, 4>() const {
            return {V.x, V.y, V.z, W};
        }

        TQuaternion<T> operator+(const TQuaternion<T> &rhs) const {
            TQuaternion<T> result;

            for (int i = 0; i < 4; i++) {
                result.values[i] = this->values[i] + rhs.values[i];
            }

            return result;
        }

        TQuaternion<T> operator-(const TQuaternion<T> &rhs) const {
            TQuaternion<T> result;

            for (int i = 0; i < 4; i++) {
                result.values[i] = this->values[i] - rhs.values[i];
            }

            return result;
        }

        TQuaternion<T> operator-() const {
            TQuaternion<T> result;

            for (int i = 0; i < 4; i++) {
                result.values[i] = -this->values[i];
            }

            return result;
        }

        TQuaternion<T> operator+() const {
            return *this;
        }

        TQuaternion<T> operator*(const TQuaternion<T> &rhs) const {
            const TVector<T, 3> v1 = cross(V, rhs.V);
            const TVector<T, 3> v2 = rhs.V * W;
            const TVector<T, 3> v3 = V * rhs.W;
            const TVector<T, 3> v = v1 + v2 + v3;

            const T w = W * rhs.W - dot(V, rhs.V);

            return {v.x, v.y, v.z, w};
        }

        TQuaternion<T> operator/(const TQuaternion<T> &rhs) const {
            return (*this) * inverse(rhs);
        }

        TQuaternion<T> operator*(const T s) const {
            TQuaternion<T> result;

            for (int i = 0; i < 4; i++) {
                result.values[i] = this->values[i] * s;
            }

            return result;
        }

        TQuaternion<T> operator/(const T s) const {
            TQuaternion<T> result;

            for (int i = 0; i < 4; i++) {
                result.values[i] = this->values[i] / s;
            }

            return result;
        }

        friend TQuaternion<T> operator*(const T s, const TQuaternion<T> &q) {
            return q * s;
        }

        TQuaternion<T> &operator+=(const TQuaternion<T> &rhs) {
            for (int i = 0; i < 4; i++) {
                this->values[i] += rhs.values[i];
            }

            return *this;
        }

        TQuaternion<T> &operator-=(const TQuaternion<T> &rhs) {
            for (int i = 0; i < 4; i++) {
                this->values[i] -= rhs.values[i];
            }

            return *this;
        }

        TQuaternion<T> &operator*=(const TQuaternion<T> &rhs) {
            *this = *this * rhs;

            return *this;
        }

        TQuaternion<T> &operator/=(const TQuaternion<T> &rhs) {
            *this = *this / rhs;

            return *this;
        }

        TQuaternion<T> &operator*=(const T s) {
            for (int i = 0; i < 4; i++) {
                this->values[i] *= s;
            }

            return *this;
        }

        TQuaternion<T> &operator/=(const T s) {
            for (int i = 0; i < 4; i++) {
                this->values[i] /= s;
            }

            return *this;
        }

        bool operator==(const TQuaternion<T> &rhs) const {
            for (int i = 0; i < 4; i++) {
                if (!equals(values[i], rhs.values[i])) {
                    return false;
                }
            }

            return true;
        }

        bool operator!=(const TQuaternion<T> &rhs) const {
            return !(*this == rhs);
        }
    };

    template <typename T = float> TQuaternion<T> conjugate(const TQuaternion<T> &q) {
        return {-q.V, q.W};
    }

    template <typename T = float> TQuaternion<T> inverse(const TQuaternion<T> &q) {
        return conjugate(q) / norm2(q);
    }

    template <typename T = float> T dot(const TQuaternion<T> &q1, const TQuaternion<T> &q2) {
        T sum = T(0);

        for (int i = 0; i < 4; i++) {
            sum += q1.values[i] * q2.values[i];
        }

        return sum;
    }

    template <typename T = float> T norm2(const TQuaternion<T> &q) {
        return dot(q, q);
    }

    /**
     * @brief Compute the magnitude, module or length (AKA Absolute Value) for a given Quaternion.
     */
    template <typename T = float> T norm(const TQuaternion<T> &q) {
        return static_cast<T>(std::sqrt(norm2(q)));
    }

    /**
     * @brief Compute a Quaternion with a unit magnitude (1)
     */
    template <typename T = float> TQuaternion<T> normalize(const TQuaternion<T> &q) {
        return q / norm(q);
    }

    template <typename T = float> TVector<T, 3> transform(const TQuaternion<T> &q, const TVector<T, 3> &v) {
        const auto inv_q = inverse(q);
        const auto q1 = q * TQuaternion<T>(v);
        const auto final_q = q1 * inv_q;

        return final_q.V;
    }

    template <typename T = float> TQuaternion<T> quatId() {
        return TQuaternion<T>({T(0), T(0), T(0)}, T(1));
    }

    template <typename T = float> TQuaternion<T> quatZero() {
        return TQuaternion<T>({T(0), T(0), T(0)}, T(0));
    }

    template <typename T = float> TQuaternion<T> quatRotationRH(const TVector<T, 3> &axis, const T radians) {
        assert(equals(norm(axis), T{1}) && "Axis should be normalized");
        const T angle = T{0.5} * radians;
        const T sin = std::sin(angle);
        const T cos = std::cos(angle);

        return TQuaternion<T>(sin * axis, cos);
    }

    template <typename T = float> TQuaternion<T> quatRotationLH(const TVector<T, 3> &axis, const T radians) {
        assert(equals(norm(axis), T{1}) && "Axis should be normalized");

        const T angle = T{0.5} * radians;

        return TQuaternion<T>(-std::sin(angle) * axis, std::cos(angle));
    }

    using Quat = TQuaternion<float>;
    using Quatd = TQuaternion<double>;

    extern template struct TQuaternion<float>;
    extern template struct TQuaternion<double>;
} // namespace xe

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif
