
#include "MainWindowQt.h"

#include "dialogs/DialogManagerQt.h"
#include "UtilitiesQt.h"
#include "widgets/FolderBrowserQt.h"
#include <QCloseEvent>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <functional>
#include <iostream>
#include <vector>
#include <xenoide/core/FileService.h>
#include <xenoide/ui/IDEFrame.h>

namespace xenoide {
    const char *appTitle = "Xenoide";

    std::string getScintillaText(ScintillaEdit *scintillaEdit) {
        const QByteArray text = scintillaEdit->getText(scintillaEdit->textLength());
        return {text.constData(), static_cast<size_t>(text.size())};
    }

    std::vector<FileFilter> getFileFilters() {
        return {FileFilter{"All Files", {"*"}}};
    }

    MainWindowQt::MainWindowQt() : dialogManager(this) {
        updateWindowTitle();
        initializeActions();
        setupMenuBar();

        // Folder Browser
        const auto areas = QFlags<Qt::DockWidgetArea>(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        folderBrowserDock = new QDockWidget("Folder Browser", this);
        folderBrowser = new FolderBrowserQt(folderBrowserDock);
        QObject::connect(folderBrowser, &FolderBrowserQt::pathActivated, this, [this](const QString &path) {
            if (checkIfCanCloseDocument() != CloseCheckResult::CanClose) {
                return;
            }

            openDocument(path.toStdString());
        });

        folderBrowser->setRootFolder("/Users/fapablaza/Desktop/devwarecl/Xenoide");
        folderBrowserDock->setAllowedAreas(areas);
        folderBrowserDock->setWidget(folderBrowser);
        this->addDockWidget(Qt::LeftDockWidgetArea, folderBrowserDock);

        // Document Manager
        documentManager = new DocumentManagerMdiQt(this);

        // Scintilla Editor
        scintillaEdit = new ScintillaEdit(this);
        QObject::connect(scintillaEdit, &ScintillaEdit::modified, this, &MainWindowQt::scintillaModified);

        scintillaEdit->styleClearAll();
        scintillaEdit->setBufferedDraw(false);
        scintillaEdit->styleSetFont(0, "Courier");
        scintillaEdit->setCaretLineVisible(true);
        scintillaEdit->setIndent(4);
        scintillaEdit->setUseTabs(false);
        scintillaEdit->hide();

        setCentralWidget(documentManager);
        documentManager->appendDocumentWindow();
        documentManager->appendDocumentWindow();
    }

    void MainWindowQt::scintillaModified(
        Scintilla::ModificationFlags type,
        Scintilla::Position position,
        Scintilla::Position length,
        Scintilla::Position linesAdded,
        const QByteArray &text,
        Scintilla::Position line,
        Scintilla::FoldLevel foldNow,
        Scintilla::FoldLevel foldPrev
    ) {

        // do other actions
        document.flags = static_cast<DocumentFlags>(document.flags | DF_MODIFIED);

        // trigger documentModifiedAction
        documentChangedAction->trigger();
    }

    QAction *MainWindowQt::createAction(const QString &text, const std::optional<QKeySequence> &keySequence) {
        auto action = new QAction(text, this);

        if (keySequence.has_value()) {
            action->setShortcut(keySequence.value());
        }

        return action;
    }

    void MainWindowQt::updateWindowTitle() {

        const std::string documentTitle = document.computeTitle();

        setWindowTitle((std::string{appTitle} + " - " + documentTitle).c_str());
    }

    QMessageBox::StandardButton MainWindowQt::askForSavingChanges() {
        constexpr auto buttons = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
        const std::string text = std::format("File {} has unsaved changed. Do you want to save it now?", document.computeFileTitle());

        return QMessageBox::question(this, appTitle, text.c_str(), buttons);
    }

    std::optional<std::filesystem::path> MainWindowQt::pickFile(const FileDialogType &type) {
        auto fileDialog = FileDialogData{};
        fileDialog.title = type == FileDialogType::SaveFile ? "Save File" : "Open File";
        fileDialog.type = type;
        fileDialog.defaultPath = document.computeFileTitle();
        fileDialog.filters = getFileFilters();

        return dialogManager.showFileDialog(fileDialog);
    }

    std::optional<std::filesystem::path> MainWindowQt::pickFolder() {
        auto folderDialog = FolderDialogData{};
        folderDialog.title = "Open Folder";

        return dialogManager.showFolderDialog(folderDialog);
    }

    void MainWindowQt::saveDocument(const std::filesystem::path &filePath) {
        const std::string fileName = filePath.string();
        const std::string content = getScintillaText(scintillaEdit);
        fileService.save(fileName, content);

        document.filePath = filePath;
        document.flags = DF_NONE;
        documentChangedAction->trigger();
    }

    void MainWindowQt::openDocument(const std::filesystem::path &filePath) {
        std::string content = fileService.load(filePath.string());
        scintillaEdit->setText(content.c_str());

        document.filePath = filePath;
        document.flags = DF_NONE;
        documentChangedAction->trigger();
    }

    CloseCheckResult MainWindowQt::checkIfCanCloseDocument() {
        if (document.flags & DF_MODIFIED) {
            QMessageBox::StandardButton selected = askForSavingChanges();

            if (selected == QMessageBox::Yes) {
                documentSaveAction->trigger();

                if (document.flags & DF_MODIFIED) {
                    return CloseCheckResult::Cancelled;
                }
            }

            if (selected == QMessageBox::Cancel) {
                return CloseCheckResult::Cancelled;
            }
        }

        return CloseCheckResult::CanClose;
    }

    void MainWindowQt::initializeActions() {
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
        documentChangedAction = createAction("");

        QObject::connect(documentChangedAction, &QAction::triggered, [this]() { updateWindowTitle(); });

        QObject::connect(documentOpenAction, &QAction::triggered, [this]() {
            if (checkIfCanCloseDocument() != CloseCheckResult::CanClose) {
                return;
            }

            if (auto filePath = pickFile(FileDialogType::OpenFile); filePath) {
                openDocument(*filePath);
            }
        });

        QObject::connect(folderOpenAction, &QAction::triggered, [this]() {
            if (checkIfCanCloseDocument() != CloseCheckResult::CanClose) {
                return;
            }

            if (const auto folderPath = pickFolder(); folderPath) {
                folderBrowser->setRootFolder(folderPath->c_str());
            }
        });

        QObject::connect(documentSaveAction, &QAction::triggered, [this]() {
            if (document.filePath) {
                saveDocument(*document.filePath);
                return;
            }

            if (auto filePath = pickFile(FileDialogType::SaveFile); filePath) {
                saveDocument(*filePath);
            }
        });

        QObject::connect(documentSaveAsAction, &QAction::triggered, [this]() {
            if (auto filePath = pickFile(FileDialogType::SaveFile); filePath) {
                saveDocument(*filePath);
            }
        });
    }

    void MainWindowQt::setupMenuBar() {
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

    void MainWindowQt::closeEvent(QCloseEvent *evt) {
        if (checkIfCanCloseDocument() == CloseCheckResult::CanClose) {
            close();
        }
    }
} // namespace xenoide
