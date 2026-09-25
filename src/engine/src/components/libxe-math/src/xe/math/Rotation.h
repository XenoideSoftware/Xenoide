
#ifndef __XE_MATH_ROTATION_HPP__
#define __XE_MATH_ROTATION_HPP__

#include "Vector.h"

namespace xe {
    template <typename T> struct Rotation {
        T angle;
        TVector<T, 3> axis;
    };

} // namespace xe

#endif
