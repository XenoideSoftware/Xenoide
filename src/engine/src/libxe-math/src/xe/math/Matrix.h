
#ifndef __XE_MATH_MATRIX_HPP__
#define __XE_MATH_MATRIX_HPP__

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <ostream>

#include "Common.h"
#include "Vector.h"

namespace xe {
    template <typename T, int R, int C> struct TMatrix;

    template <typename T, int R, int C> TMatrix<T, R, C> transpose(const TMatrix<T, R, C> &m);

    template <typename T, int R, int C> TMatrix<T, R, C> inverse(const TMatrix<T, R, C> &m);

    template <typename T, int R, int C> TMatrix<T, R, C> inverse(const TMatrix<T, R, C> &m, const T abs);

    template <typename T, int R, int C> TMatrix<T, R, C> adjoint(const TMatrix<T, R, C> &matrix);

    template <typename T, int R, int C> T determinant(const TMatrix<T, R, C> &m);

    enum class MatrixOrder { RowMajor, ColumnMajor };

    /**
     * @brief NxM matrix struct, in row-major order.
     */
    template <typename T, int R, int C> struct TMatrix {
    private:
        //! row-column accessor
        union {
            T element[R][C];
            TVector<T, C> v[R];
        };

    public:
        TMatrix() {
            for (int i = 0; i < R; i++) {
                v[i] = TVector<T, C>{};
            }
        }

        explicit TMatrix(const T *const values) {
            assert(values);
            std::memcpy(data(), values, R * C * sizeof(T));
        }

        explicit TMatrix(const std::initializer_list<T> il) {
            assert(il.size() == R * C);

            T *values = data();

            for (const T value : il) {
                *values = value;
                values++;
            }
        }

        explicit TMatrix(const std::array<TVector<T, C>, R> &rows) {
            for (int i = 0; i < R; i++) {
                for (int j = 0; j < C; j++) {
                    v[i] = rows[i];
                }
            }
        }

        explicit TMatrix(const TVector<T, R * C> &vector) {
            T *values = data();

            for (int i = 0; i < R * C; i++) {
                values[i] = vector.values[i];
            }
        }

        constexpr MatrixOrder order() const {
            return MatrixOrder::RowMajor;
        }

        bool operator==(const TMatrix<T, R, C> &other) const;

        bool operator!=(const TMatrix<T, R, C> &other) const;

        TMatrix<T, R, C> operator+() const;

        TMatrix<T, R, C> operator+(const TMatrix<T, R, C> &rhs) const;

        TMatrix<T, R, C> operator-() const;

        TMatrix<T, R, C> operator-(const TMatrix<T, R, C> &rhs) const;

        template <int R2, int C2> TMatrix<T, R, C2> operator*(const TMatrix<T, R2, C2> &rhs) const;

        TMatrix<T, R, C> operator/(const TMatrix<T, R, C> &rhs) const;

        TMatrix<T, R, C> operator*(const T s) const;

        TMatrix<T, R, C> operator/(const T s) const;

        TMatrix<T, R, C> &operator+=(const TMatrix<T, R, C> &rhs);

        TMatrix<T, R, C> &operator-=(const TMatrix<T, R, C> &rhs);

        TMatrix<T, R, C> &operator*=(const TMatrix<T, R, C> &rhs);

        TVector<T, R> operator*(const TVector<T, R> &v) const;

        inline friend TVector<T, R> operator*(const TVector<T, R> &c, const TMatrix<T, R, C> &m) {
            TVector<T, R> result;

            for (int row = 0; row < R; row++) {
                result[row] = dot(m.getColumn(row), c);
            }

            return result;
        }

        inline friend TMatrix<T, R, C> operator*(const T s, const TMatrix<T, R, C> &m) {
            return m * s;
        }

        const TVector<T, C> &operator[](const size_t i) const {
            assert(i < R);

            return v[i];
        }

        TVector<T, C> &operator[](const size_t i) {
            assert(i < R);

            return v[i];
        }

        T &operator()(const int i, const int j) {
            assert(i >= 0);
            assert(j >= 0);
            assert(i < R);
            assert(j < C);

            return element[i][j];
        }

        const T operator()(const int i, const int j) const {
            assert(i >= 0);
            assert(j >= 0);
            assert(i < R);
            assert(j < C);

            return element[i][j];
        }

        TVector<T, C> getRow(const int i) const {
            assert(i >= 0);
            assert(i < R);

            TVector<T, C> result;

            for (int j = 0; j < C; j++) {
                result.values[j] = (*this)(i, j);
            }

            return result;
        }

        TVector<T, R> getColumn(const int j) const {
            assert(j >= 0);
            assert(j < C);

            TVector<T, R> result;

            for (int i = 0; i < R; i++) {
                result.values[i] = (*this)(i, j);
            }

            return result;
        }

        TMatrix<T, R, C> &setRow(const int i, const TVector<T, C> &r) {
            for (int j = 0; j < C; j++) {
                (*this)(i, j) = r[j];
            }

            return *this;
        }

        TMatrix<T, R, C> &setColumn(const int j, const TVector<T, R> &c) {
            for (int i = 0; i < R; i++) {
                (*this)(i, j) = c[i];
            }

            return *this;
        }

        auto getSubMatrix(const int row, const int column) const {
            if constexpr (C > 2 && R > 2) {
                assert(row >= 0);
                assert(row < R);

                assert(column >= 0);
                assert(column < C);

                TMatrix<T, R - 1, C - 1> result;

                int ii = 0, jj = 0;

                for (int i = 0; i < R; ++i) {
                    if (i == row) {
                        continue;
                    }

                    for (int j = 0; j < C; ++j) {
                        if (j == column) {
                            continue;
                        }

                        result(ii, jj) = (*this)(i, j);
                        ++jj;
                    }

                    ++ii;
                    jj = 0;
                }

                return result;
            }
        }

        T *data() {
            return &element[0][0];
        }

        const T *data() const {
            return &element[0][0];
        }

    public:
        static auto columns(const std::array<TVector<T, R>, C> &columns) {
            TMatrix<T, R, C> result;

            for (int i = 0; i < R; i++) {
                for (int j = 0; j < C; j++) {
                    result.setColumn(j, columns[j]);
                }
            }

            return result;
        }

        static auto rows(const std::array<TVector<T, C>, R> &rows) {
            TMatrix<T, R, C> result;

            for (int i = 0; i < R; i++) {
                for (int j = 0; j < C; j++) {
                    result.setRow(i, rows[i]);
                }
            }

            return result;
        }

        /**
         * @brief Build a matrix initialized with zeroes.
         */
        static auto zero() {
            TMatrix<T, R, C> result;

            for (int j = 0; j < C; ++j) {
                for (int i = 0; i < R; ++i) {
                    result(i, j) = static_cast<T>(0);
                }
            }

            return result;
        }
    };

    template <typename T = float, int N> auto matIdentity() {
        constexpr auto R = N;
        constexpr auto C = N;

        auto result = TMatrix<T, R, C>();

        for (int i = 0; i < R; ++i) {
            result(i, i) = static_cast<T>(1);
        }

        return result;
    }

    template <typename T = float> auto mat2Identity() {
        return matIdentity<T, 2>();
    }

    template <typename T = float> auto mat3Identity() {
        return matIdentity<T, 3>();
    }

    template <typename T = float> auto mat4Identity() {
        return matIdentity<T, 4>();
    }

    template <typename T, int N> auto matScaling(const TVector<T, N> &scale) {
        auto result = matIdentity<T, N>();

        for (int i = 0; i < N; ++i) {
            result(i, i) = scale[i];
        }

        return result;
    }

    template <typename T = float> auto mat2Scaling(const TVector<T, 2> &scale) {
        return matScaling<T, 2>(scale);
    }

    template <typename T = float> auto mat3Scaling(const TVector<T, 3> &scale) {
        return matScaling<T, 3>(scale);
    }

    template <typename T = float> auto mat4Scaling(const TVector<T, 4> &scale) {
        return matScaling<T, 4>(scale);
    }

    template <typename T, int N> auto matTranslation(const TVector<T, N - 1> &displace) {
        constexpr auto R = N;
        constexpr auto C = N;

        auto result = matIdentity<T, N>();

        for (int i = 0; i < R - 1; i++) {
            result[i][C - 1] = displace[i];
        }

        return result;
    }

    template <typename T = float> auto mat3Translation(const TVector<T, 2> &displace) {
        return matTranslation<T, 3>(displace);
    }

    template <typename T = float> auto mat4Translation(const TVector<T, 3> &displace) {
        return matTranslation<T, 4>(displace);
    }

    template <typename T, int N> auto matRotationX(const T radians) {
        if constexpr (N == 3 || N == 4) {
            auto result = matIdentity<T, N>();

            const T cos = std::cos(radians);
            const T sin = std::sin(radians);

            result(1, 1) = cos;
            result(2, 2) = cos;

            result(1, 2) = -sin;
            result(2, 1) = sin;

            return result;
        }
    }

    template <typename T = float> auto mat3RotationX(const T radians) {
        return matRotationX<T, 3>(radians);
    }

    template <typename T = float> auto mat4RotationX(const T radians) {
        return matRotationX<T, 4>(radians);
    }

    template <typename T, int N> auto matRotationY(const T radians) {
        if constexpr (N == 3 || N == 4) {
            auto result = matIdentity<T, N>();

            const T cos = std::cos(radians);
            const T sin = std::sin(radians);

            result(0, 0) = cos;
            result(2, 2) = cos;
            result(0, 2) = sin;
            result(2, 0) = -sin;

            return result;
        }
    }

    template <typename T = float> auto mat3RotationY(const T radians) {
        return matRotationY<T, 3>(radians);
    }

    template <typename T = float> auto mat4RotationY(const T radians) {
        return matRotationY<T, 4>(radians);
    }

    template <typename T, int N> auto matRotationZ(const T radians) {
        if constexpr (N == 2 || N == 3 || N == 4) {
            auto result = matIdentity<T, N>();

            const T cos = std::cos(radians);
            const T sin = std::sin(radians);

            result(0, 0) = cos;
            result(1, 1) = cos;
            result(0, 1) = -sin;
            result(1, 0) = sin;

            return result;
        }
    }

    template <typename T = float> auto mat2Rotation(const T radians) {
        return matRotationZ<T, 2>(radians);
    }

    template <typename T = float> auto mat3RotationZ(const T radians) {
        return matRotationZ<T, 3>(radians);
    }

    template <typename T = float> auto mat4RotationZ(const T radians) {
        return matRotationZ<T, 4>(radians);
    }

    template <typename T, int N> static auto matRotation(const T rads, const TVector<T, 3> &axis) {
        if constexpr (N == 3 || N == 4) {
            assert(!std::isnan(rads));
            assert(!std::isinf(rads));

            const auto I = matIdentity<T, 3>();

            const T cos = std::cos(rads);
            const T sin = std::sin(rads);

            TVector<T, 3> V = normalize(axis);

            const auto c1 = TVector<T, 3>{static_cast<T>(0), -V.z, V.y};
            const auto c2 = TVector<T, 3>{V.z, static_cast<T>(0), -V.x};
            const auto c3 = TVector<T, 3>{-V.y, V.x, static_cast<T>(0)};

            const auto matS = TMatrix<T, 3, 3>::columns({c1, c2, c3});

            const auto matUUT = TMatrix<T, 3, 1>{V} * TMatrix<T, 1, 3>{V};
            const auto tempResult = matUUT + cos * (I - matUUT) + sin * matS;

            auto result = matIdentity<T, N>();

            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    result(i, j) = tempResult(i, j);
                }
            }

            return xe::transpose(result);
        }
    }

    template <typename T = float> auto mat3Rotation(const T rads, const TVector<T, 3> &axis) {
        return matRotation<T, 3>(rads, axis);
    }

    template <typename T = float> auto mat4Rotation(const T rads, const TVector<T, 3> &axis) {
        return matRotation<T, 4>(rads, axis);
    }

    template <typename T = float> auto mat4LookAtRH(const TVector<T, 3> &eye, const TVector<T, 3> &at, const TVector<T, 3> &up) {
        const auto zaxis = normalize(at - eye);
        const auto xaxis = normalize(cross(zaxis, up));
        const auto yaxis = cross(xaxis, zaxis);

        auto result = TMatrix<T, 4, 4>{};
        result[0] = {xaxis.x, xaxis.y, xaxis.z, -dot(xaxis, eye)};
        result[1] = {yaxis.x, yaxis.y, yaxis.z, -dot(yaxis, eye)};
        result[2] = {-zaxis.x, -zaxis.y, -zaxis.z, dot(zaxis, eye)};
        result[3] = {T(0), T(0), T(0), T(1)};

        return result;
    }

    template <typename T = float> auto mat4Perspective(const T fov_radians, const T aspect, const T znear, const T zfar) {
        assert(fov_radians > T(0));
        assert(aspect > T(0));
        assert(znear > T(0));
        assert(zfar > znear);

        const T half_fov = fov_radians / T(2);
        const T f = T(1) / std::tan(half_fov);
        const T zdiff = znear - zfar;
        const T a = aspect;

        auto result = TMatrix<T, 4, 4>{};
        result[0] = TVector<T, 4>{f / a, T(0), T(0), T(0)};
        result[1] = TVector<T, 4>{T(0), f, T(0), T(0)};
        result[2] = TVector<T, 4>{T(0), T(0), (zfar + znear) / zdiff, (T(2) * zfar * znear) / zdiff};
        result[3] = TVector<T, 4>{T(0), T(0), T(-1), T(0)};

        return result;
    }

    template <typename T = float> auto mat4Ortho(const TVector<T, 3> &pmin, const TVector<T, 3> &pmax) {
        constexpr auto two = static_cast<T>(2);
        constexpr auto one = static_cast<T>(1);
        const auto diff = pmax - pmin;

        auto result = matIdentity<T, 4>();

        result(0, 0) = two / diff.x;
        result(1, 1) = two / diff.y;
        result(2, 2) = -two / diff.z;
        result(3, 3) = one;

        result(0, 3) = -(pmax.x + pmin.x) / diff.x;
        result(1, 3) = -(pmax.y + pmin.y) / diff.y;
        result(2, 3) = -(pmax.z + pmin.z) / diff.z;

        return result;
    }

    extern template struct TMatrix<float, 2, 2>;
    extern template struct TMatrix<float, 3, 3>;
    extern template struct TMatrix<float, 4, 4>;

    using Matrix2 = TMatrix<float, 2, 2>;
    using Matrix3 = TMatrix<float, 3, 3>;
    using Matrix4 = TMatrix<float, 4, 4>;

    template <typename T, int R, int C> bool TMatrix<T, R, C>::operator==(const TMatrix<T, R, C> &other) const {
        const T *values = data();
        const T *rhs_values = other.data();

        for (int i = 0; i < R * C; i++) {
            if (!equals(values[i], rhs_values[i])) {
                return false;
            }
        }

        return true;
    }

    template <typename T, int R, int C> bool TMatrix<T, R, C>::operator!=(const TMatrix<T, R, C> &other) const {
        return !(*this == other);
    }

    template <typename T, int R, int C> TMatrix<T, R, C> TMatrix<T, R, C>::operator+(const TMatrix<T, R, C> &rhs) const {
        TMatrix<T, R, C> result;

        T *result_values = result.data();

        const T *lhs_values = data();
        const T *rhs_values = rhs.data();

        for (int i = 0; i < R * C; i++) {
            result_values[i] = lhs_values[i] + rhs_values[i];
        }

        return result;
    }

    template <typename T, int R, int C> TMatrix<T, R, C> TMatrix<T, R, C>::operator-() const {
        TMatrix<T, R, C> result;
        T *result_values = result.data();

        const T *lhs_values = data();

        for (int i = 0; i < R * C; i++) {
            result_values[i] = -lhs_values[i];
        }

        return result;
    }

    template <typename T, int R, int C> TMatrix<T, R, C> TMatrix<T, R, C>::operator+() const {
        return *this;
    }

    template <typename T, int R, int C> TMatrix<T, R, C> TMatrix<T, R, C>::operator-(const TMatrix<T, R, C> &rhs) const {
        TMatrix<T, R, C> result;

        T *result_values = result.data();

        const T *lhs_values = data();
        const T *rhs_values = rhs.data();

        for (int i = 0; i < R * C; i++) {
            result_values[i] = lhs_values[i] - rhs_values[i];
        }

        return result;
    }

    template <typename T, int R, int C> template <int R2, int C2> TMatrix<T, R, C2> TMatrix<T, R, C>::operator*(const TMatrix<T, R2, C2> &rhs) const {
        static_assert(R == C2);
        static_assert(C == R2);

        TMatrix<T, R, C2> result;

        for (int i = 0; i < R; i++) {
            for (int j = 0; j < C2; j++) {
                const auto rowI = getRow(i);
                const auto colJ = rhs.getColumn(j);
                const auto mIJ = dot(rowI, colJ);

                result(i, j) = mIJ;
            }
        }

        return result;
    }

    template <typename T, int R, int C> TMatrix<T, R, C> TMatrix<T, R, C>::operator/(const TMatrix<T, R, C> &rhs) const {
        return *this * inverse(rhs);
    }

    template <typename T, int R, int C> TMatrix<T, R, C> TMatrix<T, R, C>::operator*(const T s) const {
        TMatrix<T, R, C> result;

        T *result_values = result.data();
        const T *values = data();

        for (int i = 0; i < R * C; i++) {
            result_values[i] = values[i] * s;
        }

        return result;
    }

    template <typename T, int R, int C> TMatrix<T, R, C> TMatrix<T, R, C>::operator/(const T s) const {
        TMatrix<T, R, C> result;

        T *result_values = result.data();
        const T *values = data();

        for (int i = 0; i < R * C; i++) {
            result_values[i] = values[i] / s;
        }

        return result;
    }

    template <typename T, int R, int C> TMatrix<T, R, C> &TMatrix<T, R, C>::operator+=(const TMatrix<T, R, C> &rhs) {
        const T *rhs_values = rhs.data();
        T *values = data();

        for (int i = 0; i < R * C; i++) {
            values[i] += rhs_values[i];
        }

        return *this;
    }

    template <typename T, int R, int C> TMatrix<T, R, C> &TMatrix<T, R, C>::operator-=(const TMatrix<T, R, C> &rhs) {
        const T *rhs_values = rhs.data();
        T *values = data();

        for (int i = 0; i < R * C; i++) {
            values[i] -= rhs_values[i];
        }

        return *this;
    }

    template <typename T, int R, int C> TMatrix<T, R, C> &TMatrix<T, R, C>::operator*=(const TMatrix<T, R, C> &rhs) {
        *this = *this * rhs;

        return *this;
    }

    template <typename T, int R, int C> TVector<T, R> TMatrix<T, R, C>::operator*(const TVector<T, R> &c) const {
        TVector<T, R> result;

        for (int row = 0; row < R; row++) {
            result[row] = dot(getRow(row), c);
        }

        return result;
    }

    template <typename T, int R, int C> TMatrix<T, R, C> transpose(const TMatrix<T, R, C> &m) {
        TMatrix<T, C, R> result;

        for (int i = 0; i < R; i++) {
            for (int j = 0; j < C; j++) {
                result(j, i) = m(i, j);
            }
        }

        return result;
    }

    template <typename T, int N> TMatrix<T, N, N> transpose(const TMatrix<T, N, N> &m) {
        return transpose<T, N, N>(m);
    }

    template <typename T, int R, int C> TMatrix<T, R, C> inverse(const TMatrix<T, R, C> &m) {
        return transpose(adjoint(m)) / determinant(m);
    }

    template <typename T, int R, int C> TMatrix<T, R, C> inverse(const TMatrix<T, R, C> &m, const T det) {
        return transpose(adjoint(m)) / det;
    }

    template <typename T, int R, int C> T determinant(const TMatrix<T, R, C> &m) {
        if constexpr (R == C && R > 0) {
            if constexpr (R == 1) {
                return m(0, 0);
            }
            if constexpr (R == 2) {
                return m(1, 1) * m(0, 0) - m(0, 1) * m(1, 0);
            } else {
                T result = T(0);

                const int i = 0;

                for (int j = 0; j < C; j++) {
                    const T factor = (j + 1) % 2 ? T(-1) : T(1);
                    const T subdet = determinant(m.getSubMatrix(i, j));

                    result += factor * m(i, j) * subdet;
                }

                return result;
            }
        }
    }

    template <typename T, int R, int C> TMatrix<T, R, C> adjoint(const TMatrix<T, R, C> &matrix) {
        if constexpr (C == 2 && R == 2) {
            TMatrix<T, R, C> result;

            result(0, 0) = matrix(1, 1);
            result(0, 1) = -matrix(0, 1);
            result(1, 0) = -matrix(1, 0);
            result(1, 1) = matrix(0, 0);

            return result;
        } else {
            TMatrix<T, R, C> result;

            for (int i = 0; i < R; ++i) {
                for (int j = 0; j < C; ++j) {
                    const T factor = ((i + j) % 2 == 1) ? static_cast<T>(1) : static_cast<T>(-1);
                    result(i, j) = factor * determinant(matrix.getSubMatrix(i, j));
                }
            }

            return result;
        }
    }

    template <typename T, int R, int C> std::ostream &operator<<(std::ostream &os, const xe::TMatrix<T, R, C> &m) {
        os << "xe::TMatrix<" << typeid(T).name() << ", " << R << ", " << C << "> {" << std::endl;

        for (int i = 0; i < R; ++i) {
            os << "  ";
            os << "{ ";
            for (int j = 0; j < C; j++) {
                os << std::fixed << std::setprecision(8) << std::setw(12) << m(i, j);

                if (j < C - 1) {
                    os << ", ";
                }
            }
            os << " }";

            if (i < R - 1) {
                os << ",";
            }

            os << std::endl;
        }

        os << "}" << std::endl;

        return os;
    }

    using mat4 = Matrix4;

    template<typename T, int R, int C>
    using tmat = TMatrix<T, R, C>;

} // namespace xe

#endif
