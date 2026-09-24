
#pragma once

#include <algorithm>
#include <cassert>
#include <ostream>

namespace xe {
    /**
     * @brief Mathematical Range pair implementation.
     *
     * This struct represent Ranges of the form:
     * [min, max)
     *
     * @tparam T
     */
    template <typename T> struct TRange {
        T min = static_cast<T>(0);
        T max = static_cast<T>(1);

        TRange() {
        }

        explicit TRange(const T value) : min(value), max(value) {
        }

        explicit TRange(const T value1, const T value2) : TRange(value1) {
            expand(value2);
        }

        void expand(const T value) {
            min = std::min(min, value);
            max = std::max(max, value);
        }

        bool overlap(const TRange<T> &other) const {
            return partialOverlap(other) || other.partialOverlap(*this);
        }

        /**
         * @brief Checks if the current Projection overlaps with the supplied Projection.
         *
         * @param other
         * @return true
         * @return false
         */
        bool partialOverlap(const TRange<T> &other) const {
            assert(max >= min);
            assert(other.max >= other.min);

            if (min >= other.min && min < other.max) {
                return true;
            }

            if (max > other.min && max < other.max) {
                return true;
            }

            return false;
        }
    };

    /**
     * @brief Serializes a Range<T> using the supplied ostream
     */
    template <typename T> inline std::ostream &operator<<(std::ostream &os, const TRange<T> &range) {
        os << "xe::Range<" << typeid(T).name() << ">{ ";

        os << range.min << ", ";
        os << range.max << " }";

        return os;
    }

    using Range = TRange<float>;
    using Ranged = TRange<double>;
    using Rangei = TRange<int>;

    extern template struct TRange<float>;
    extern template struct TRange<double>;
    extern template struct TRange<int>;
} // namespace xe
