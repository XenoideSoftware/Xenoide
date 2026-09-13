/**
 * @file VectorExpr.h
 * @brief A xe::Vector implementation using expression templates for high-performance
 */

#ifndef _XE_MATH_VECTOREXPR_HPP__
#define _XE_MATH_VECTOREXPR_HPP__

#if defined(_MSC_VER)
#pragma message("This is an experimental header file. Although it contains usable code, it is incomplete, unstable, and has not been tested yet. Use at your own risk")
#else
#pragma message "This is an experimental header file. Although it contains usable code, it is incomplete, unstable, and has not been tested yet. Use at your own risk"
#endif

#include <cmath>
#include <functional>

namespace xe {
    /**
     * @brief Computes the sum of all
     */
    template <typename VectorExpr> auto sum(VectorExpr expression) {
        auto result = expression[0];

        for (int i = 1; i < VectorExpr::vector_type::size; i++) {
            result += expression[i];
        }

        return result;
    }

    template <typename VectorExprLeft, typename VectorExprRight> auto dot(VectorExprLeft v1, VectorExprRight v2) {
        return sum(v1 * v2);
    }

    /*
    template<typename VectorExprLeft, typename VectorExprRight>
    auto cross(VectorExprLeft v1, VectorExprRight v2) {
        constexpr int size = VectorExprLeft::vector_type::size;

        if constexpr (size == 3) {
            return
        }
    }
    */

    template <typename VectorExpr> auto norm2(VectorExpr v) {
        return sum(v * v);
    }

    template <typename VectorExpr> auto norm(VectorExpr v) {
        return std::sqrt(norm2(v));
    }

    template <typename VectorExprLeft, typename VectorExprRight, typename BinaryOperator> class VectorBinaryExpr {
    public:
        typedef typename VectorExprLeft::vector_type vector_type;

    public:
        VectorBinaryExpr(VectorExprLeft left, VectorExprRight right) : m_left(left), m_right(right) {
        }

        auto operator[](const int i) const {
            return m_operator(m_left[i], m_right[i]);
        }

    private:
        VectorExprLeft m_left;
        VectorExprRight m_right;
        BinaryOperator m_operator = BinaryOperator();
    };

    template <typename VectorExpr, typename UnaryOperator> class VectorUnaryExpr {
    public:
        typedef typename VectorExpr::vector_type vector_type;

    public:
        VectorUnaryExpr(VectorExpr vector) : m_vector(vector) {
        }

        auto operator[](const int i) const {
            return m_operator(m_vector[i]);
        }

    private:
        VectorExpr m_vector;
        UnaryOperator m_operator = UnaryOperator();
    };

    template <typename T> struct identity {
        T operator()(const T value) const {
            return value;
        }
    };

    template <typename VectorExpr> auto operator+(VectorExpr expression) {
        return VectorUnaryExpr<VectorExpr, identity<typename VectorExpr::vector_type::type>>(expression);
    }

    template <typename T> struct negate {
        T operator()(const T value) const {
            return -value;
        }
    };

    template <typename VectorExpr> auto operator-(VectorExpr expression) {
        return VectorUnaryExpr<VectorExpr, negate<typename VectorExpr::vector_type::type>>(expression);
    }

    template <typename VectorExprLeft, typename VectorExprRight> auto operator+(VectorExprLeft left, VectorExprRight right) {
        return VectorBinaryExpr<VectorExprLeft, VectorExprRight, std::plus<typename VectorExprLeft::vector_type::type>>(left, right);
    }

    template <typename VectorExprLeft, typename VectorExprRight> auto operator-(VectorExprLeft left, VectorExprRight right) {
        return VectorBinaryExpr<VectorExprLeft, VectorExprRight, std::minus<typename VectorExprLeft::vector_type::type>>(left, right);
    }

    template <typename VectorExprLeft, typename VectorExprRight> auto operator*(VectorExprLeft left, VectorExprRight right) {
        return VectorBinaryExpr<VectorExprLeft, VectorExprRight, std::multiplies<typename VectorExprLeft::vector_type::type>>(left, right);
    }

    template <typename VectorExprLeft, typename VectorExprRight> auto operator/(VectorExprLeft left, VectorExprRight right) {
        return VectorBinaryExpr<VectorExprLeft, VectorExprRight, std::divides<typename VectorExprLeft::vector_type::type>>(left, right);
    }
} // namespace xe

#endif