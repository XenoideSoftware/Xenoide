
#pragma once

#include <string>
#include <optional>
#include <filesystem>
#include <xenoide/core/Predef.h>
#include <xenoide/ui/Menu.h>

namespace xenoide {
    class FolderBrowser;
    class FolderBrowserModel;
    class DialogManager;
    class FolderService;
    struct Point;

    class ActionBus;
    class FolderBrowserPresenter {
    public:
        FolderBrowserPresenter(ActionBus *actionBus, FolderBrowserModel *model);

        void onInitialized(FolderBrowser *folderBrowser, DialogManager *dialogManager);

        virtual ~FolderBrowserPresenter();

        virtual void onBrowseFolder();

        virtual void onCreateFile();

        virtual void onCreateFolder();

        virtual void onMoveSelectedPath(const std::string &targetFolder);

        virtual void onRenameSelectedPath();

        virtual void onOpenSelectedFile();

        virtual void onDeleteSelectedPath();

        virtual void onDisplayFolder(const std::filesystem::path &folderPath);

        virtual void onContextMenuRequested(const Point &point);

    private:
        std::optional<std::string> askValidPath(const std::string &title, const std::string &prompt, const std::string &promptForInvalidInput, const std::string &defaultValue);

        DialogManager *dialogView;
        FolderBrowser *view;
        FolderBrowserModel *model;
        ActionBus *actionBus = nullptr;
    };

} // namespace xenoide