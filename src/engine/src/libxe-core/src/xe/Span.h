
#include <iterator>

namespace xe {

    template<typename T>
    class Span {
    public:
        struct Iterator {
            using iterator_category = std::forward_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T*;
            using reference = T&;

            explicit Iterator(pointer ptr) : _ptr(ptr) {}

            reference operator*() const {
                return *_ptr;
            }

            pointer operator->() {
                return _ptr;
            }

            Iterator& operator++() {
                _ptr++;

                return *this;
            }

            Iterator& operator++(int) {
                Iterator it = *this;

                _ptr++;

                return it;
            }

            friend bool operator==(const Iterator &lhs, const Iterator &rhs) {
                return _ptr == rhs._ptr;
            }

            friend bool operator==(const Iterator &lhs, const Iterator &rhs) {
                return _ptr != rhs._ptr;
            }

        private:
            pointer _ptr;
        };

        explicit Span() {}

        explicit Span(T* data, size_t size) : _data(data), _size(size) {
            // prevents issues where data and size have inconsistencies
            assert(data == nullptr && size == 0 || data != nullptr && size > 0);
        }

        T* data() {
            return _data;
        }

        size_t size() const {
            return _size;
        }

        explicit operator bool() const {
            return _data != nullptr;
        }

        Iterator begin() {
            return Iterator(_data);
        }

        Iterator end() {
            return Iterator(_data + _size);
        }

        T& operator[](const size_t i) {
            assert(i < _size);

            return _data[i];
        }

        T operator[](const size_t i) const {
            assert(i < _size);

            return _data[i];
        }

    private:
        T* _data = nullptr;
        size_t _size = 0;
    };

    // utility factory function
    namespace detail {
        using std::data;
        using std::size;

        template<typename C>
        auto adl_data(C&& c) -> decltype(data(std::forward<C>(c))) {
            return data(std::forward<C>(c));
        }

        template<typename C>
        auto adl_size(C&& c) -> decltype(size(std::forward<C>(c))) {
            return size(std::forward<C>(c));
        }
    }

    template<typename C>
    auto span(C&& c) -> decltype (
        Span<std::remove_pointer_t<decltype(detail::adl_data(c))>> (
            detail::adl_data(c),
            detail::adl_size(c)
        )) {

        using T = std::remove_pointer_t<decltype(detail::adl_data(c))>;

        return Span<T> (
            detail::adl_data(c),
            detail::adl_size(c)
        );
    }

    // only for completeness
    template<typename T>
    Span<T> span(T *data, size_t size) {
        return Span<T>(data, size);
    }
}
