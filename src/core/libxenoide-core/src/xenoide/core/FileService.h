
#pragma once

#include <string>
#include <vector>
#include <functional>

namespace xenoide {
    using FileSystemVisitor = std::function<bool(const std::string &path)>;

    class FileService {
    public:
        virtual ~FileService() {
        }

        virtual std::string load(const std::string &filePath);

        virtual void save(const std::string &filePath, const std::string &content);

        virtual void touch(const std::string &filePath);

        virtual bool exists(const std::string &path) const;

        virtual void enumerateIntoVisitor(const std::string &folder, FileSystemVisitor visitor);

        virtual std::vector<std::string> enumerate(const std::string &folder);

        virtual std::string extractName(const std::string &path) const;

        virtual std::vector<std::string> listChildFolders(const std::string &folderPath) const;
    };
} // namespace xenoide
