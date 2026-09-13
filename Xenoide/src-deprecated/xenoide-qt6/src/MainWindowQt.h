
#pragma once

#ifndef __XENOIDE_UI_QT5_MAINWINDOW_HPP__
#define __XENOIDE_UI_QT5_MAINWINDOW_HPP__

#include <map>

#include <QMainWindow>
#include <QDockWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <ScintillaEdit.h>
#include <xenoide/ui/AppModel.h>
#include <xenoide/core/FileService.h>

#include "dialogs/DialogManagerQt.h"
#include "widgets/DocumentManagerMdiQt.h"

namespace xenoide {
    enum class CloseCheckResult {
        Cancelled,
        CanClose
    };

    class FolderBrowserQt;
    class MainWindowQt : public QMainWindow {
        Q_OBJECT;

    public:
        explicit MainWindowQt();

    private:
        void closeEvent(QCloseEvent *evt) override;

        void setupMenuBar();

        void updateWindowTitle();

        QAction *
        createAction(const QString &text,
                     const std::optional<QKeySequence> &keySequence = {});

        void initializeActions();

        void scintillaModified(Scintilla::ModificationFlags type, Scintilla::Position position, Scintilla::Position length, Scintilla::Position linesAdded,
          const QByteArray &text, Scintilla::Position line, Scintilla::FoldLevel foldNow, Scintilla::FoldLevel foldPrev);

        QMessageBox::StandardButton askForSavingChanges();

        std::optional<std::filesystem::path> pickFile(const FileDialogType &type);

        std::optional<std::filesystem::path> pickFolder();

        void saveDocument(const std::filesystem::path &filePath);

        void openDocument(const std::filesystem::path &filePath);

        CloseCheckResult checkIfCanCloseDocument();

        Document2 document;
        ScintillaEdit *scintillaEdit = nullptr;

        QAction *documentNewAction = nullptr;
        QAction *documentOpenAction = nullptr;
        QAction *documentSaveAction = nullptr;
        QAction *documentSaveAsAction = nullptr;
        QAction *appExitAction = nullptr;
        QAction *selectionCutAction = nullptr;
        QAction *selectionCopyAction = nullptr;
        QAction *selectionPasteAction = nullptr;
        QAction *appAboutAction = nullptr;
        // triggers when the document was modified by any means
        QAction *documentChangedAction = nullptr;

        QAction *folderOpenAction = nullptr;

        QMenuBar *menuBar = nullptr;
        QMenu *fileMenu = nullptr;
        QMenu *editMenu = nullptr;
        QMenu *helpMenu = nullptr;

        QDockWidget *folderBrowserDock;
        FolderBrowserQt *folderBrowser;

        DocumentManagerMdiQt *documentManager = nullptr;

        DialogManagerQt dialogManager;

        FileService fileService;
    };
}

#endif
