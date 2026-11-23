
#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <xenoide/qt6/QCodeEditor.h>
#include <xenoide/CodeEditorPresenter.h>
#include <xenoide/core/FileService.h>
#include <gsl-lite/gsl-lite.hpp>

namespace xenoide {
    class MainWindow : public QMainWindow {
        Q_OBJECT;

    public:
        explicit MainWindow();

    private:
        void closeEvent(QCloseEvent *evt) override;

        void setupMenuBar();

        QAction *createAction(const QString &text, const std::optional<QKeySequence> &keySequence = {});

        void initializeActions();

        xenoide::qt6::QCodeEditor *codeEditor = nullptr;
        QMenuBar *menuBar = nullptr;
        QMenu *fileMenu = nullptr;
        QMenu *editMenu = nullptr;
        QMenu *helpMenu = nullptr;

        QAction *documentNewAction = nullptr;
        QAction *documentOpenAction = nullptr;
        QAction *folderOpenAction = nullptr;
        QAction *documentSaveAction = nullptr;
        QAction *documentSaveAsAction = nullptr;
        QAction *appExitAction = nullptr;
        QAction *selectionCutAction = nullptr;
        QAction *selectionCopyAction = nullptr;
        QAction *selectionPasteAction = nullptr;
        QAction *appAboutAction = nullptr;

        FileService fileService;
        CodeEditorPresenter presenter{gsl_lite::not_null<FileService *>{&fileService}};
    };
} // namespace xenoide
