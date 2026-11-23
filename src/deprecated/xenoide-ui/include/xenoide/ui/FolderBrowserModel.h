
#pragma once

#include <string>
#include <optional>
#include <filesystem>
#include <xenoide/core/Predef.h>
#include <xenoide/ui/Menu.h>

namespace xenoide {
    class FolderBrowser;
    class DialogManager;
    class FolderService;

    class FolderBrowserModel {
    public:
        explicit FolderBrowserModel(FolderService *folderService);
        ~FolderBrowserModel();

        void setCurrentFolderPath(const std::filesystem::path &folderPath);

        std::filesystem::path getCurrentFolderPath() const;

        std::vector<std::filesystem::path> listChildPaths(const std::filesystem::path &folderPath) const;

        FolderService *folderService;
        std::filesystem::path currentFolderPath;
    };

} // namespace xenoide