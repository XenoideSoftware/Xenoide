
#ifndef __XENOIDE_UI_FOLDERSERVICE_HPP__
#define __XENOIDE_UI_FOLDERSERVICE_HPP__

#include <vector>
#include <string>
#include <filesystem>

namespace Xenoide {
    class FolderService {
    public:
        virtual ~FolderService() {
        }

        virtual std::vector<std::filesystem::path> listChildFolders(const std::filesystem::path &folderPath) const;
    };
} // namespace Xenoide

#endif
