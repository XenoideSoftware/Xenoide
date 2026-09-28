#ifndef XE_CMAKE_CORE_SOURCE_SPAN_H
#define XE_CMAKE_CORE_SOURCE_SPAN_H

#include <cstddef>

namespace xe::cmake::core {

    // Byte-exact source span over a listfile buffer. Offsets are 0-based while
    // line/column coordinates are 1-based. A span [start_offset, end_offset) is
    // half-open.
    struct SourceSpan {
        std::size_t start_offset = 0;
        std::size_t end_offset = 0;
        std::size_t start_line = 1;
        std::size_t start_column = 1;
        std::size_t end_line = 1;
        std::size_t end_column = 1;

        static SourceSpan invalid() {
            SourceSpan span;
            span.start_line = 0;
            span.start_column = 0;
            span.end_line = 0;
            span.end_column = 0;
            return span;
        }

        bool is_valid() const {
            return start_offset <= end_offset && start_line >= 1 && end_line >= 1 && start_line <= end_line;
        }

        bool contains(const SourceSpan &other) const {
            return start_offset <= other.start_offset && other.end_offset <= end_offset;
        }

        bool overlaps(const SourceSpan &other) const {
            return start_offset < other.end_offset && other.start_offset < end_offset;
        }

        bool operator==(const SourceSpan &other) const {
            return start_offset == other.start_offset && end_offset == other.end_offset;
        }

        bool operator!=(const SourceSpan &other) const {
            return !(*this == other);
        }
    };

} // namespace xe::cmake::core

#endif // XE_CMAKE_CORE_SOURCE_SPAN_H