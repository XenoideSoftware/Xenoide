
#pragma once 

#include "Model.h"
#include "FileSystemService.h"

#include <vector>
#include <set>
#include <map>

namespace Xenoide {
	class FileSystemService;

    using FolderExplorerCallback = std::function<void(const Path, const std::string)>;


	struct FolderExplorerItem {
		std::string title;
		Path path;
	};


	class FolderExplorer {
	public:
		explicit FolderExplorer(FileSystemService* fileSystemService);

		Folder getFolder() const {
			return rootFolder;
		}

		void setFolder(const Folder rootFolder, FolderExplorerCallback callback);

		int compare(const int itemId1, const int itemId2) const;

		bool itemIsPopulated(const int itemId) const;

		void insertItem(const int itemId, const Path path);

		void populateItem(const int parentItemId, FolderExplorerCallback callback);

		std::vector<FolderExplorerItem> populateItem(const int parentItemId);

	private:
		FileSystemService* fileSystemService = nullptr;
		Folder rootFolder;
		// boost::bimap<int, Xenoide::Path> pathItemsCache;
        std::map<int, Xenoide::Path> pathItemsCacheLeft;
        std::map<Xenoide::Path, int> pathItemsCacheRight;
		std::set<int> populatedItems;
	};
}
