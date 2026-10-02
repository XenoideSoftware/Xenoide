
#ifndef __XE_MATH_RECT_HPP__
#define __XE_MATH_RECT_HPP__

#include "Boundary.h"

namespace xe {
    typedef TBoundary<float, 2> Rectf;
    typedef TBoundary<int, 2> Recti;

    extern template class TBoundary<float, 2>;
    extern template class TBoundary<double, 2>;
    extern template class TBoundary<int, 2>;
} // namespace xe

#endif
