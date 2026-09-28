#ifndef XE_CMAKE_IO_FILE_SYSTEM_H
#define XE_CMAKE_IO_FILE_SYSTEM_H

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace xe::cmake::io {

    // Filesystem abstraction so the checker can run against physical disks or an
    // in-memory virtual filesystem. All paths are forward-slash normalized.
    class IFileSystem {
    public:
        virtual ~IFileSystem() = default;

        virtual bool exists(std::string_view path) const = 0;
        virtual bool is_directory(std::string_view path) const = 0;
        virtual bool is_file(std::string_view path) const = 0;
        virtual bool create_directories(std::string_view path) = 0;
        virtual bool read_file(std::string_view path, std::string &out_content) const = 0;
        virtual bool write_file(std::string_view path, std::string_view content) const = 0;
        virtual std::vector<std::string> list_directory(std::string_view path) const = 0;
    };

    // Physical filesystem backed by std::filesystem.
    class NativeFileSystem final : public IFileSystem {
    public:
        bool exists(std::string_view path) const override;
        bool is_directory(std::string_view path) const override;
        bool is_file(std::string_view path) const override;
        bool create_directories(std::string_view path) override;
        bool read_file(std::string_view path, std::string &out_content) const override;
        bool write_file(std::string_view path, std::string_view content) const override;
        std::vector<std::string> list_directory(std::string_view path) const override;
    };

    // In-memory virtual filesystem used by the high-performance test suites and
    // the end-to-end integration tests.
    class InMemoryFileSystem final : public IFileSystem {
    public:
        bool exists(std::string_view path) const override;
        bool is_directory(std::string_view path) const override;
        bool is_file(std::string_view path) const override;
        bool create_directories(std::string_view path) override;
        bool read_file(std::string_view path, std::string &out_content) const override;
        bool write_file(std::string_view path, std::string_view content) const override;
        std::vector<std::string> list_directory(std::string_view path) const override;

    private:
        struct Entry {
            bool is_dir = false;
            std::string content;
        };

        // Keys are normalized without a leading slash.
        std::vector<std::pair<std::string, Entry>> entries_;

        Entry *find_entry(std::string_view key);
        const Entry *find_entry(std::string_view key) const;
    };

} // namespace xe::cmake::io

#endif // XE_CMAKE_IO_FILE_SYSTEM_H