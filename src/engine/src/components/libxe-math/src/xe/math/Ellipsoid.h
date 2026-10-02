
#ifndef __XE_MATH_ELLIPSOID_HPP__
#define __XE_MATH_ELLIPSOID_HPP__

#include "Vector.h"

#include <limits>

namespace xe {
    /**
     * @brief Ellipsoid in the 3-space
     */
    template <typename T> struct TEllipsoid {
        TVector<T, 3> center;
        TVector<T, 3> size;

        //! Initializes an Ellipsoid at the origin with radius 1, 1, 1.
        TEllipsoid() : center{T(0), T(0), T(0)}, size{T(1), T(1), T(1)} {
        }

        //! Initializes a Ellipsoid from the given center and radius
        TEllipsoid(const TVector<T, 3> &center, const TVector<T, 3> &size) : center(center), size(size) {
        }
    };

    //! Serializes the content of a Box object to an ostream.
    template <typename T> inline std::ostream &operator<<(std::ostream &os, const TEllipsoid<T> &ellipsoid) {
        os << "xe::Ellipsoid<" << typeid(T).name() << "{ " << std::endl;
        os << "    " << ellipsoid.center << ", " << std::endl;
        os << "    " << ellipsoid.size << std::endl;
        os << "}" << std::endl;

        return os;
    }

    extern template struct TEllipsoid<float>;
    extern template struct TEllipsoid<double>;

    using Ellipsoid = TEllipsoid<float>;
    using Ellipsoidd = TEllipsoid<double>;
} // namespace xe

#endif
