
#ifndef XE_FLAGS_H
#define XE_FLAGS_H

namespace XE {
    // usage:
    // using MyFlags = Flags<MyEnumBits>;
    template <typename T> class Flags {
    public:
        Flags() {
        }

        explicit Flags(const T value) : value{value} {
        }

        bool operator==(const Flags<T> &rhs) const {
            return value == rhs.value;
        }

        bool operator!=(const Flags<T> &rhs) const {
            return value != rhs.value;
        }

        Flags<T> operator&(const Flags<T> &rhs) const {
            return value & rhs.value;
        }

        Flags<T> operator|(const Flags<T> &rhs) const {
            return value | rhs.value;
        }

        Flags<T> operator^(const Flags<T> &rhs) const {
            return value ^ rhs.value;
        }

        Flags<T> &operator&=(const Flags<T> &rhs) {
            value &= rhs.value;

            return *this;
        }

        Flags<T> &operator|=(const Flags<T> &rhs) {
            value | rhs.value;

            return *this;
        }

        Flags<T> &operator^=(const Flags<T> &rhs) {
            value ^= rhs.value;

            return *this;
        }

    private:
        T value{};
    };
} // namespace XE

#endif // XE_FLAGS_H
