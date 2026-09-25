
#ifndef __XE_MATH_RAY_HPP__
#define __XE_MATH_RAY_HPP__

#include "Vector.h"
#include <cassert>

namespace xe {
    /**
     * @brief Ray with normalized direction in the 3-space
     */
    template <typename T> struct TRay {
        TVector<T, 3> position;
        TVector<T, 3> direction;

        /**
         * @brief Computes the point of the ray at 't' distance from the starting point, to the
         * direction of the ray.
         */
        TVector<T, 3> pointAt(const T t) const {
            return position + t * direction;
        }

        /**
         * @brief Initializes the ray, positioned around the origin and aiming to the positive Z-axis.
         */
        TRay() {
            position = {T(0.0), T(0.0), T(0.0)};
            direction = {T(0.0), T(0.0), T(1.0)};
        }
    };

    using Ray = TRay<float>;
    using Rayd = TRay<double>;

    extern template struct TRay<float>;
    extern template struct TRay<double>;
} // namespace xe

#endif
