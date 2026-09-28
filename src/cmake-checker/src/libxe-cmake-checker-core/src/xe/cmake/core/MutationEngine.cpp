#include "MutationEngine.h"

#include <algorithm>

namespace xe::cmake::core {

    std::string TextSplicer::splice(const std::string &source, const std::vector<TextEdit> &edits) {
        if (edits.empty()) {
            return source;
        }

        std::vector<TextEdit> sorted = edits;
        std::sort(sorted.begin(), sorted.end(), [](const TextEdit &left, const TextEdit &right) {
            const std::size_t left_start = left.span.start_offset;
            const std::size_t right_start = right.span.start_offset;
            if (left_start != right_start) {
                return left_start > right_start;
            }
            return left.span.end_offset > right.span.end_offset;
        });

        std::string result;
        result.reserve(source.size() + sorted.size() * 8);
        result.append(source);

        for (const TextEdit &edit : sorted) {
            switch (edit.kind) {
            case TextEdit::Kind::Replace: {
                result.replace(edit.span.start_offset, edit.span.end_offset - edit.span.start_offset, edit.new_text);
                break;
            }
            case TextEdit::Kind::Remove: {
                result.erase(edit.span.start_offset, edit.span.end_offset - edit.span.start_offset);
                break;
            }
            case TextEdit::Kind::InsertBefore: {
                result.insert(edit.span.start_offset, edit.new_text);
                break;
            }
            case TextEdit::Kind::InsertAfter: {
                result.insert(edit.span.end_offset, edit.new_text);
                break;
            }
            }
        }

        return result;
    }

} // namespace xe::cmake::core