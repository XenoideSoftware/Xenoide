
#include "Vector.h"

namespace xe {
    template struct TVector<std::int32_t, 2>;
    template struct TVector<std::int32_t, 3>;
    template struct TVector<std::int32_t, 4>;

    template struct TVector<float, 2>;
    template struct TVector<float, 3>;
    template struct TVector<float, 4>;

    template struct TVector<double, 2>;
    template struct TVector<double, 3>;
    template struct TVector<double, 4>;
} // namespace xe
