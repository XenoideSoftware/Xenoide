#include "FolderBrowserPresenter.h"

#include "FolderBrowser.h"

#include <iostream>
#include <xenoide/core/FileService.h>

namespace xenoide {
    FolderBrowserPresenter::FolderBrowserPresenter(gsl_lite::not_null<FileService *> fileService) : fileService(fileService), folderExplorer(fileService) {
    }

    void FolderBrowserPresenter::onInitialized(gsl_lite::not_null<FolderBrowser *> newView) {
        view = newView;
    }

    void FolderBrowserPresenter::openFolder(const std::filesystem::path &folderPath) {
        std::cout << "FolderBrowserPresenter::openFolder" << std::endl;

        currentFolder = folderPath;
        itemIdCounter = 0;
        itemPaths.clear();
        view->clearItems();

        const Folder folder{folderPath.string()};

        // setFolder calls the callback synchronously with (rootPath, displayName).
        // We register the root item inside the callback so that FolderExplorer's
        // internal cache is populated before loadChildren is called below.
        int rootId = -1;
        folderExplorer.setFolder(folder, [this, &rootId](const Path &rootPath, const std::string &name) {
            rootId = registerItem(rootPath);
            view->addRootItem(rootId, name, rootPath.value);
        });

        if (rootId >= 0) {
            loadChildren(rootId);
            // Expand after children are loaded so that an itemExpanded signal
            // fired by the view does not re-trigger lazy loading.
            view->expandItem(rootId);
        }
    }

    void FolderBrowserPresenter::onItemExpanded(int itemId) {
        std::cout << "FolderBrowserPresenter::onItemExpanded" << std::endl;
        // Guard against re-population and against expanding file items
        // (which should never happen in practice, but protects FolderExplorer asserts).
        if (folderExplorer.itemIsPopulated(itemId))
            return;

        const auto it = itemPaths.find(itemId);
        if (it == itemPaths.end() || !it->second.isFolder())
            return;

        loadChildren(itemId);
    }

    bool FolderBrowserPresenter::hasFolder() const {
        std::cout << "FolderBrowserPresenter::hasFolder" << std::endl;
        return !currentFolder.empty();
    }

    std::filesystem::path FolderBrowserPresenter::getFolder() const {
        std::cout << "FolderBrowserPresenter::hasFolder" << std::endl;

        return currentFolder;
    }

    int FolderBrowserPresenter::registerItem(const Path &path) {
        std::cout << "FolderBrowserPresenter::registerItem" << std::endl;

        const int id = itemIdCounter++;
        folderExplorer.insertItem(id, path);
        itemPaths[id] = path;
        return id;
    }

    void FolderBrowserPresenter::loadChildren(int parentId) {
        std::cout << "FolderBrowserPresenter::loadChildren" << std::endl;

        for (const auto &item : folderExplorer.populateItem(parentId)) {
            const int childId = registerItem(item.path);
            view->addChildItem(parentId, childId, item.title, item.path.value, item.path.isFolder());
        }
    }
} // namespace xenoide
