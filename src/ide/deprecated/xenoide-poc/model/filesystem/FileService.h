
#ifndef __XENOIDE_UI_FILESERVICE_HPP__
#define __XENOIDE_UI_FILESERVICE_HPP__

#include <memory>
#include <string>
#include <filesystem>

namespace Xenoide {
    class FileService {
    public:
        virtual ~FileService() {
        }

        virtual std::string load(const std::filesystem::path &filePath);

        virtual void save(const std::filesystem::path &filePath, const std::string &content);

        virtual void touch(const std::filesystem::path &filePath);
    };
} // namespace Xenoide

#endif
