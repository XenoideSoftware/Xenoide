
#include <xenoide/core/FolderService.h>

#include <filesystem>


namespace Xenoide {
    std::vector<std::filesystem::path> FolderService::listChildFolders(const std::filesystem::path &folderPath) const {
        auto childPathVector = std::vector<std::filesystem::path>{};

        auto subPathIterator = std::filesystem::directory_iterator{folderPath};
        auto end = std::filesystem::directory_iterator{};

        while (subPathIterator != end) {
            std::filesystem::path subPath = subPathIterator->path();

            childPathVector.push_back(subPath);

            subPathIterator++;
        }

        return childPathVector;
    }
}
