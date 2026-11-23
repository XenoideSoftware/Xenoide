#pragma once

#include <filesystem>
#include <unordered_map>

#include <gsl-lite/gsl-lite.hpp>

#include <xenoide/core/FolderExplorer.h>
#include <xenoide/core/Model.h>

namespace xenoide {
    class FileService;
    class FolderBrowser;

    class FolderBrowserPresenter {
    public:
        explicit FolderBrowserPresenter(gsl_lite::not_null<FileService *> fileService);

        /** Wire the view. Must be called before any other method. */
        void onInitialized(gsl_lite::not_null<FolderBrowser *> view);

        /**
         * Load and display the contents of folderPath.
         * Clears any previously displayed tree, populates the root item
         * and its immediate children, then expands the root.
         */
        void openFolder(const std::filesystem::path &folderPath);

        /**
         * Called by the view when the user expands a tree node.
         * Lazily populates the node's children on the first expansion.
         */
        void onItemExpanded(int itemId);

        [[nodiscard]] bool hasFolder() const;
        [[nodiscard]] std::filesystem::path getFolder() const;

    private:
        /** Allocate a new ID, register the path in FolderExplorer and the local map. */
        int registerItem(const Path &path);

        /** Enumerate path's children via FolderExplorer and add them to the view. */
        void loadChildren(int parentId);

        gsl_lite::not_null<FileService *> fileService;
        FolderBrowser *view = nullptr;
        FolderExplorer folderExplorer;
        int itemIdCounter = 0;
        std::unordered_map<int, Path> itemPaths;
        std::filesystem::path currentFolder;
    };
} // namespace xenoide
