
#pragma once

#include <string>
#include <vector>
#include <memory>


namespace xenoide {
class FileSearchDialogModel;
class FileSearchDialog;
  class FileSearchDialogPresenter {
  public:
    FileSearchDialogPresenter(FileSearchDialogModel *model);

    void onInitialized(FileSearchDialog *view);

    void onAccepted(const std::string &filePath);

    void onCancelled();

    void onFilenameFilterRequested(const std::string &fileNamePart);

  private:
    FileSearchDialogModel *model = nullptr;
    FileSearchDialog *view = nullptr;
  };
}
