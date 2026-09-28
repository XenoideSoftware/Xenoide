#include "FileSystem.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace xe::cmake::io {

    namespace {

        std::string normalize_key(std::string_view path) {
            std::string key(path);
            while (!key.empty() && key.front() == '/') {
                key.erase(key.begin());
            }
            return key;
        }

    } // namespace

    bool NativeFileSystem::exists(std::string_view path) const {
        return std::filesystem::exists(std::filesystem::path(path));
    }

    bool NativeFileSystem::is_directory(std::string_view path) const {
        return std::filesystem::is_directory(std::filesystem::path(path));
    }

    bool NativeFileSystem::is_file(std::string_view path) const {
        return std::filesystem::is_regular_file(std::filesystem::path(path));
    }

    bool NativeFileSystem::create_directories(std::string_view path) {
        std::error_code ec;
        std::filesystem::create_directories(std::filesystem::path(path), ec);
        return !ec;
    }

    bool NativeFileSystem::read_file(std::string_view path, std::string &out_content) const {
        std::ifstream stream(std::filesystem::path(path), std::ios::binary);
        if (!stream) {
            return false;
        }
        std::ostringstream buffer;
        buffer << stream.rdbuf();
        out_content = buffer.str();
        return true;
    }

    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    bool NativeFileSystem::write_file(std::string_view path, std::string_view content) const {
        std::ofstream stream(std::filesystem::path(path), std::ios::binary | std::ios::trunc);
        if (!stream) {
            return false;
        }
        stream.write(content.data(), static_cast<std::streamsize>(content.size()));
        return static_cast<bool>(stream);
    }

    std::vector<std::string> NativeFileSystem::list_directory(std::string_view path) const {
        std::vector<std::string> entries;
        std::error_code ec;
        for (const auto &entry : std::filesystem::directory_iterator(std::filesystem::path(path), ec)) {
            entries.push_back(entry.path().filename().string());
        }
        return entries;
    }

    bool InMemoryFileSystem::exists(std::string_view path) const {
        return find_entry(normalize_key(path)) != nullptr;
    }

    bool InMemoryFileSystem::is_directory(std::string_view path) const {
        const Entry *entry = find_entry(normalize_key(path));
        if (entry != nullptr) {
            return entry->is_dir;
        }
        // A path is a directory when some stored entry lives beneath it.
        const std::string key = normalize_key(path);
        for (const auto &pair : entries_) {
            if (pair.first.size() > key.size() && pair.first.substr(0, key.size()) == key && (!key.empty() && pair.first[key.size()] == '/')) {
                return true;
            }
        }
        return false;
    }

    bool InMemoryFileSystem::is_file(std::string_view path) const {
        const Entry *entry = find_entry(normalize_key(path));
        return entry != nullptr && !entry->is_dir;
    }

    bool InMemoryFileSystem::create_directories(std::string_view path) {
        const std::string key = normalize_key(path);
        if (find_entry(key) != nullptr) {
            return true;
        }
        Entry entry;
        entry.is_dir = true;
        entries_.push_back(std::make_pair(key, entry));
        return true;
    }

    bool InMemoryFileSystem::read_file(std::string_view path, std::string &out_content) const {
        const Entry *entry = find_entry(normalize_key(path));
        if (entry == nullptr || entry->is_dir) {
            return false;
        }
        out_content = entry->content;
        return true;
    }

    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    bool InMemoryFileSystem::write_file(std::string_view path, std::string_view content) const {
        InMemoryFileSystem *self = const_cast<InMemoryFileSystem *>(this);
        const std::string key = normalize_key(path);
        Entry *entry = self->find_entry(key);
        if (entry == nullptr) {
            Entry new_entry;
            new_entry.is_dir = false;
            new_entry.content = std::string(content);
            self->entries_.push_back(std::make_pair(key, new_entry));
            return true;
        }
        if (entry->is_dir) {
            return false;
        }
        entry->content = std::string(content);
        return true;
    }

    std::vector<std::string> InMemoryFileSystem::list_directory(std::string_view path) const {
        const std::string key = normalize_key(path);
        std::vector<std::string> entries;
        for (const auto &pair : entries_) {
            const std::string &entry_key = pair.first;
            if (entry_key.size() <= key.size()) {
                continue;
            }
            if (entry_key.substr(0, key.size()) != key) {
                continue;
            }
            // The next character after the directory prefix must be the separator.
            if (!key.empty() && entry_key[key.size()] != '/') {
                continue;
            }
            const std::size_t rest_start = key.empty() ? 0 : key.size() + 1;
            const std::string rest = entry_key.substr(rest_start);
            const std::size_t slash = rest.find('/');
            const std::string child = (slash == std::string::npos) ? rest : rest.substr(0, slash);
            if (child.empty()) {
                continue;
            }
            if (std::find(entries.begin(), entries.end(), child) == entries.end()) {
                entries.push_back(child);
            }
        }
        return entries;
    }

    InMemoryFileSystem::Entry *InMemoryFileSystem::find_entry(std::string_view key) {
        for (auto &pair : entries_) {
            if (pair.first == key) {
                return &pair.second;
            }
        }
        return nullptr;
    }

    const InMemoryFileSystem::Entry *InMemoryFileSystem::find_entry(std::string_view key) const {
        for (const auto &pair : entries_) {
            if (pair.first == key) {
                return &pair.second;
            }
        }
        return nullptr;
    }

} // namespace xe::cmake::io