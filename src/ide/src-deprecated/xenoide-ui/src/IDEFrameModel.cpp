

#include <xenoide/ui/IDEFrameModel.h>

#include <iostream>
#include <cassert>
#include <filesystem>
#include <optional>

#include <xenoide/ui/Menu.h>
#include <xenoide/ui/MenuPanel.h>
#include <xenoide/ui/FolderBrowser.h>
#include <xenoide/ui/FolderBrowserModel.h>
#include <xenoide/ui/DocumentManagerPresenter.h>
#include <xenoide/ui/DocumentManagerModel.h>

namespace xenoide {
    IDEFrameModel::IDEFrameModel() {
        this->folderService = std::make_unique<FolderService>();
        this->documentManagerModel = std::make_unique<DocumentManagerModel>();
        this->folderBrowserModel = std::make_unique<FolderBrowserModel>(this->folderService.get());
    }

    IDEFrameModel::~IDEFrameModel() {
    }

    std::vector<FileFilter> IDEFrameModel::getFileFilters() const {
        return {
            {"All Files", {"*.*"}},
            {"C/C++ Files", {"*.hpp", "*.cpp", "*.hh", "*.cc", "*.h", "*.c"}},
        };
    }

    DocumentManagerModel *IDEFrameModel::getDocumentManagerModel() {
        return documentManagerModel.get();
    }

    FolderBrowserModel *IDEFrameModel::getFolderBrowserModel() {
        return folderBrowserModel.get();
    }

    std::optional<std::filesystem::path> IDEFrameModel::getWorkspaceFolder() const {
        return this->workspaceFolder;
    }

    void IDEFrameModel::setWorkspaceFolder(std::filesystem::path workspaceFolder) {
        std::cout << "setWorkspaceFolder(" << workspaceFolder.string() << ")" << std::endl;
        this->workspaceFolder = workspaceFolder;
    }
} // namespace xenoide
