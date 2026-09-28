#ifndef XE_CMAKE_IO_SYNC_WRITER_H
#define XE_CMAKE_IO_SYNC_WRITER_H

#include "FileSystem.h"

#include "xe/cmake/core/MutationEngine.h"

#include <string>
#include <string_view>
#include <vector>

namespace xe::cmake::io {

    // Applies a WorkspaceEdit transactionally to a filesystem. Each file's edits
    // are spliced into the current buffer content; a file is only written when its
    // content actually changed.
    class SyncWriter {
    public:
        explicit SyncWriter(const IFileSystem &filesystem) : filesystem_(filesystem) {
        }

        // Returns the files that were actually written.
        std::vector<std::string> apply(const xe::cmake::core::WorkspaceEdit &edit) const;

    private:
        const IFileSystem &filesystem_;
    };

} // namespace xe::cmake::io

#endif // XE_CMAKE_IO_SYNC_WRITER_H