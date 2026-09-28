#include "SyncWriter.h"

namespace xe::cmake::io {

    std::vector<std::string> SyncWriter::apply(const xe::cmake::core::WorkspaceEdit &edit) const {
        std::vector<std::string> written;
        for (const auto &file_edit : edit.files) {
            std::string current;
            if (!filesystem_.read_file(file_edit.file_path, current)) {
                continue;
            }
            const std::string updated = xe::cmake::core::TextSplicer::splice(current, file_edit.edits);
            if (updated == current) {
                continue;
            }
            if (filesystem_.write_file(file_edit.file_path, updated)) {
                written.push_back(file_edit.file_path);
            }
        }
        return written;
    }

} // namespace xe::cmake::io