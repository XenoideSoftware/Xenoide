
#ifndef __XE_MATH_BOX_HPP__
#define __XE_MATH_BOX_HPP__

#include "Boundary.h"

namespace xe {
    extern template class TBoundary<float, 3>;
    extern template class TBoundary<double, 3>;
    extern template class TBoundary<int, 3>;

    //! Bounding box in 3-space. It's a Boundary specialization
    template <typename T> using TBox = TBoundary<T, 3>;

    using Box = TBox<float>;
    using Boxd = TBox<double>;
    using Boxi = TBox<int>;

    //! Serializes the content of a Box object to an ostream.
    template <typename T> inline std::ostream &operator<<(std::ostream &os, const TBox<T> &box) {
        os << "xe::Box<" << typeid(T).name() << ">{ " << std::endl;
        os << "    " << box.getMinEdge() << ", " << std::endl;
        os << "    " << box.getMaxEdge() << std::endl;
        os << "}" << std::endl;

        return os;
    }
} // namespace xe

#endif
