
#include "QXenoideMainWindow.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <gsl-lite/gsl-lite.hpp>
#include <xenoide/CodeEditor.h>

namespace xenoide {
    MainWindow::MainWindow() {
        initializeActions();
        setupMenuBar();

        codeEditor = new xenoide::qt6::QCodeEditor(this);
        setCentralWidget(codeEditor);

        presenter.onInitialized(gsl_lite::not_null<CodeEditor *>{codeEditor});

        connect(codeEditor, &xenoide::qt6::QCodeEditor::contentModified, this, [this]() { presenter.onContentChanged(); });

        connect(codeEditor, &xenoide::qt6::QCodeEditor::titleChanged, this, [this](const QString &title) { setWindowTitle(title); });

        connect(documentNewAction, &QAction::triggered, this, [this]() { presenter.onNew(); });

        connect(documentOpenAction, &QAction::triggered, this, [this]() {
            const QString fileName = QFileDialog::getOpenFileName(this, "Open File");
            if (!fileName.isEmpty()) {
                presenter.openFile(fileName.toStdString());
            }
        });

        connect(documentSaveAction, &QAction::triggered, this, [this]() {
            if (presenter.hasFilePath()) {
                presenter.onSave();
            } else {
                const QString fileName = QFileDialog::getSaveFileName(this, "Save File");
                if (!fileName.isEmpty()) {
                    presenter.onSaveAs(fileName.toStdString());
                }
            }
        });

        connect(documentSaveAsAction, &QAction::triggered, this, [this]() {
            const QString fileName = QFileDialog::getSaveFileName(this, "Save File As");
            if (!fileName.isEmpty()) {
                presenter.onSaveAs(fileName.toStdString());
            }
        });

        connect(appExitAction, &QAction::triggered, this, [this]() { close(); });
    }

    QAction *MainWindow::createAction(const QString &text, const std::optional<QKeySequence> &keySequence) {
        auto action = new QAction(text, this);

        if (keySequence.has_value()) {
            action->setShortcut(keySequence.value());
        }

        return action;
    }

    void MainWindow::initializeActions() {
        documentNewAction = createAction("&New", QKeySequence("Ctrl+N"));
        documentOpenAction = createAction("&Open ...", QKeySequence("Ctrl+O"));
        folderOpenAction = createAction("Open &Folder ...", QKeySequence("Ctrl+Shift+O"));
        documentSaveAction = createAction("&Save", QKeySequence("Ctrl+S"));
        documentSaveAsAction = createAction("Save &As ...");
        appExitAction = createAction("&Exit");
        selectionCutAction = createAction("&Cut");
        selectionCopyAction = createAction("C&opy");
        selectionPasteAction = createAction("&Paste");
        appAboutAction = createAction("&About", QKeySequence("F1"));
    }

    void MainWindow::setupMenuBar() {
        menuBar = new QMenuBar(this);
        fileMenu = menuBar->addMenu("&File");
        fileMenu->addAction(documentNewAction);
        fileMenu->addSeparator();
        fileMenu->addAction(documentOpenAction);
        fileMenu->addAction(folderOpenAction);
        fileMenu->addSeparator();
        fileMenu->addAction(documentSaveAction);
        fileMenu->addAction(documentSaveAsAction);
        fileMenu->addSeparator();
        fileMenu->addAction(appExitAction);

        editMenu = menuBar->addMenu("&Edit");
        editMenu->addAction(selectionCutAction);
        editMenu->addAction(selectionCopyAction);
        editMenu->addAction(selectionPasteAction);

        helpMenu = menuBar->addMenu("&Help");
        helpMenu->addAction(appAboutAction);

        setMenuBar(menuBar);
    }

    void MainWindow::closeEvent(QCloseEvent *evt) {
        if (presenter.isModified()) {
            const auto result = QMessageBox::question(
                this,
                "Unsaved Changes",
                "You have unsaved changes. What would you like to do?",
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                QMessageBox::Save
            );

            switch (result) {
            case QMessageBox::Save:
                if (presenter.hasFilePath()) {
                    presenter.onSave();
                } else {
                    const QString fileName = QFileDialog::getSaveFileName(this, "Save File");
                    if (fileName.isEmpty()) {
                        evt->ignore();
                        return;
                    }
                    presenter.onSaveAs(fileName.toStdString());
                }
                evt->accept();
                break;
            case QMessageBox::Discard:
                evt->accept();
                break;
            case QMessageBox::Cancel:
            default:
                evt->ignore();
                break;
            }
        } else {
            evt->accept();
        }
    }
} // namespace xenoide
