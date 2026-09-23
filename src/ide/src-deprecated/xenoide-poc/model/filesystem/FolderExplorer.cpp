
#include <xenoide/core/FolderExplorer.h>
#include <xenoide/core/FileSystemService.h>

#include <cassert>

namespace Xenoide {
    FolderExplorer::FolderExplorer(FileSystemService *fileSystemService) : fileSystemService(fileSystemService) {
        assert(fileSystemService);
    }

    void FolderExplorer::setFolder(const Folder rootFolder, FolderExplorerCallback callback) {
        pathItemsCacheLeft.clear();
        pathItemsCacheRight.clear();
        populatedItems.clear();

        this->rootFolder = rootFolder;

        const Path folderPath{PathType::Folder, rootFolder.path};
        const std::string name{fileSystemService->extractName(folderPath)};

        callback(folderPath, name);
    }

    int FolderExplorer::compare(const int itemId1, const int itemId2) const {
        const auto it1 = pathItemsCacheLeft.find(itemId1);
        const auto it2 = pathItemsCacheLeft.find(itemId2);
        const auto end = pathItemsCacheLeft.end();

        assert(it1 != end && it2 != end);

        const Path path1 = it1->second;
        const Path path2 = it2->second;

        return path1.compare(path2).value;
    }

    bool FolderExplorer::itemIsPopulated(const int itemId) const {
        const auto it = populatedItems.find(itemId);

        return it != populatedItems.end();
    }

    void FolderExplorer::insertItem(const int itemId, const Path path) {
        pathItemsCacheLeft.insert({itemId, path});
        pathItemsCacheRight.insert({path, itemId});
    }

    std::vector<FolderExplorerItem> FolderExplorer::populateItem(const int parentItemId) {
        std::vector<FolderExplorerItem> items;

        const auto folderPathIt = pathItemsCacheLeft.find(parentItemId);

        assert(folderPathIt != pathItemsCacheLeft.end());
        assert(folderPathIt->second.isFolder());

        const Folder folder{folderPathIt->second.value};
        const Path folderPath{PathType::Folder, folder.path};

        const std::vector<Path> children = fileSystemService->enumerate(folder);

        for (const Path path : children) {
            const auto parentCacheIt = pathItemsCacheRight.find(folderPath);

            if (parentCacheIt == pathItemsCacheRight.end()) {
                continue;
            }

            const std::string name = fileSystemService->extractName(path);

            items.push_back(FolderExplorerItem{name, path});
        }

        populatedItems.insert(parentItemId);

        return items;
    }

    void FolderExplorer::populateItem(const int parentItemId, FolderExplorerCallback callback) {
        const auto folderPathIt = pathItemsCacheLeft.find(parentItemId);

        assert(folderPathIt != pathItemsCacheLeft.end());
        assert(folderPathIt->second.isFolder());

        const Folder folder{folderPathIt->second.value};
        const Path folderPath{PathType::Folder, folder.path};

        const std::vector<Path> children = fileSystemService->enumerate(folder);

        for (const Path path : children) {
            const auto parentCacheIt = pathItemsCacheRight.find(folderPath);

            if (parentCacheIt == pathItemsCacheRight.end()) {
                continue;
            }

            const std::string name = fileSystemService->extractName(path);
            callback(path, name);
        }

        populatedItems.insert(parentItemId);
    }
} // namespace Xenoide
