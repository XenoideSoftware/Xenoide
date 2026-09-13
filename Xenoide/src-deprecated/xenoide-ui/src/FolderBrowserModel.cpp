
#include <xenoide/ui/FolderBrowserModel.h>


#include <iostream>
#include <fstream>
#include <filesystem>


#include <xenoide/ui/DialogManager.h>
#include <xenoide/ui/Menu.h>
#include <xenoide/ui/IDEFrame.h>

namespace xenoide {
    FolderBrowserModel::FolderBrowserModel(FolderService *folderService) {
        this->folderService = folderService;
        this->currentFolderPath = std::filesystem::current_path();
    }

    void FolderBrowserModel::setCurrentFolderPath(const std::filesystem::path &folderPath) {
        currentFolderPath = folderPath;
    }

    std::filesystem::path FolderBrowserModel::getCurrentFolderPath() const {
        return currentFolderPath;
    }

    std::vector<std::filesystem::path> FolderBrowserModel::listChildPaths(const std::filesystem::path &folderPath) const{
        return folderService->listChildFolders(folderPath);
    }

    FolderBrowserModel::~FolderBrowserModel() {}
}
