
#ifndef __XENOIDE_UI_MAINWINDOWVIEW_HPP__
#define __XENOIDE_UI_MAINWINDOWVIEW_HPP__

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
    class IDEFramePresenter;

    class IDEFrame {
    public:
        enum Panel { FOLDER_BROWSER };

        explicit IDEFrame(IDEFramePresenter *presenter);

        virtual ~IDEFrame();

        virtual DocumentManager *getDocumentManager() = 0;

        virtual DialogManager *getDialogManager() = 0;

        virtual FolderBrowser *getFolderBrowser() = 0;

        virtual void close() = 0;

        virtual void show() = 0;

        virtual void showPanel(const Panel panel) = 0;

    protected:
        IDEFramePresenter *mPresenter;
    };
} // namespace xenoide

#endif
