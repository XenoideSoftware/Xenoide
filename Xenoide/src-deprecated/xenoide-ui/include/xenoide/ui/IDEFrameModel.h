
#pragma once

#include <xenoide/core/Predef.h>
#include <xenoide/ui/FileFilter.h>
#include <xenoide/ui/DialogManager.h>
#include <xenoide/ui/Document.h>
#include <xenoide/ui/DocumentManager.h>
#include <xenoide/ui/FolderBrowser.h>
#include <xenoide/ui/FolderBrowser.h>
#include <optional>
#include <filesystem>
#include <iostream>
#include <cassert>

#include <xenoide/ui/Menu.h>
#include <xenoide/ui/MenuPanel.h>
#include <xenoide/ui/FolderBrowser.h>
#include <xenoide/ui/DocumentManagerPresenter.h>
#include <xenoide/ui/DocumentManagerModel.h>
#include <xenoide/ui/DocumentModel.h>


namespace xenoide {
  class DocumentManagerPresenter;
  class DocumentManagerModel;
  class DialogManager;
  class FolderBrowser;
  class MenuPanel;
  class IDEFramePresenter;
  class FolderBrowserModel;

  class IDEFrameModel {
  public:
    IDEFrameModel();

    ~IDEFrameModel();

    std::vector<FileFilter> getFileFilters() const;

    DocumentManagerModel *getDocumentManagerModel();

    FolderBrowserModel *getFolderBrowserModel();

    std::optional<std::filesystem::path> getWorkspaceFolder() const;

    void setWorkspaceFolder(std::filesystem::path workspaceFolder);

  private:
    std::unique_ptr<FolderService> folderService;
    std::unique_ptr<DocumentManagerModel> documentManagerModel;
    std::unique_ptr<FolderBrowserModel> folderBrowserModel;
    std::optional<std::filesystem::path> workspaceFolder;
  };
}
