#include "FixConflictResolver.h"

#include <algorithm>

namespace xe::cmake::rules {

    FixConflictResolver::Result FixConflictResolver::resolve(const std::vector<xe::cmake::core::Finding> &findings) const {
        Result result;

        for (std::size_t i = 0; i < findings.size(); ++i) {
            const xe::cmake::core::Finding &finding = findings[i];
            if (!finding.has_fix() || finding.fix->edits.empty()) {
                result.manual_indices.push_back(i);
                continue;
            }

            // Gather existing edits for this file.
            xe::cmake::core::WorkspaceEdit::FileEdit *file = nullptr;
            for (xe::cmake::core::WorkspaceEdit::FileEdit &candidate : result.workspace.files) {
                if (candidate.file_path == finding.file_path) {
                    file = &candidate;
                    break;
                }
            }
            if (file == nullptr) {
                result.workspace.files.push_back(xe::cmake::core::WorkspaceEdit::FileEdit());
                file = &result.workspace.files.back();
                file->file_path = finding.file_path;
            }

            // Reject overlapping edits.
            bool overlaps = false;
            for (const xe::cmake::core::TextEdit &existing : file->edits) {
                for (const xe::cmake::core::TextEdit &candidate : finding.fix->edits) {
                    if (existing.span.overlaps(candidate.span)) {
                        overlaps = true;
                        break;
                    }
                }
                if (overlaps) {
                    break;
                }
            }
            if (overlaps) {
                result.manual_indices.push_back(i);
                continue;
            }

            for (const xe::cmake::core::TextEdit &edit : finding.fix->edits) {
                file->edits.push_back(edit);
            }
            result.applied_indices.push_back(i);
        }

        // Sort every file's edits in reverse-offset order.
        for (xe::cmake::core::WorkspaceEdit::FileEdit &file_edit : result.workspace.files) {
            std::sort(file_edit.edits.begin(), file_edit.edits.end(), [](const xe::cmake::core::TextEdit &left, const xe::cmake::core::TextEdit &right) {
                if (left.span.start_offset != right.span.start_offset) {
                    return left.span.start_offset > right.span.start_offset;
                }
                return left.span.end_offset > right.span.end_offset;
            });
        }

        return result;
    }

} // namespace xe::cmake::rules