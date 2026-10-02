
#pragma once

#include <xenoide/core/Predef.h>
#include <xenoide/ui/FileFilter.h>
#include <xenoide/ui/DialogManager.h>
#include <xenoide/ui/Document.h>
#include <xenoide/ui/DocumentManager.h>
#include <xenoide/ui/FolderBrowser.h>

namespace xenoide {
    class DocumentManagerPresenter;
    class DocumentManagerModel;
    class DialogManager;
    class FolderBrowser;
    class MenuPanel;
    class IDEFrameModel;
    class IDEFrame;
    class ActionBus;

    class IDEFramePresenter {
    public:
        IDEFramePresenter(ActionBus *actionBus, IDEFrameModel *model);

        virtual ~IDEFramePresenter();

        void onInitialized(IDEFrame *view, DialogManager *dialogView, MenuPanel *menuView);

    public:
        void onFileNew();

        void onFileOpen();

        void onFileOpen(const std::string &fileName);

        void onFileOpenFolder();

        void onFileSave();

        void onFileSaveAs();

        void onFileSaveAll();

        void onFileClose();

        void onFileExit();

        void onEditUndo();

        void onEditRedo();

        void onEditCut();

        void onEditCopy();

        void onEditPaste();

        void onViewFolderBrowser();

        void onToolsFileSearch();

        bool onCloseRequested();

    public:
        DocumentManagerPresenter *getDocumentManagerPresenter();

        FolderBrowserPresenter *getFolderBrowserPresenter();

        // TODO: Make it private (used in Main.cpp)
        void openFolder(const std::string &fullPath);

    private:
        DialogManager *dialogView = nullptr;
        MenuPanel *menuView = nullptr;
        IDEFrame *view = nullptr;
        IDEFrameModel *model = nullptr;

        std::unique_ptr<DocumentManagerPresenter> documentManagerPresenter;
        std::unique_ptr<FolderBrowserPresenter> folderBrowserPresenter;
    };

} // namespace xenoide