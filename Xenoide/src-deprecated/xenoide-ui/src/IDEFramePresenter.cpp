
#include "xenoide/ui/ActionBus.h"

#include <xenoide/ui/IDEFramePresenter.h>

#include <xenoide/ui/IDEFrameModel.h>
#include <xenoide/ui/IDEFrame.h>
#include <xenoide/ui/DocumentManagerPresenter.h>
#include <xenoide/ui/MenuPanel.h>
#include <xenoide/ui/FolderBrowserPresenter.h>

namespace xenoide {
    IDEFramePresenter::IDEFramePresenter(ActionBus *actionBus, IDEFrameModel *model) {
        this->model = model;

        // initialize child presenters
        documentManagerPresenter = std::make_unique<DocumentManagerPresenter>(model->getDocumentManagerModel());
        folderBrowserPresenter = std::make_unique<FolderBrowserPresenter>(actionBus, model->getFolderBrowserModel());
    }

    IDEFramePresenter::~IDEFramePresenter() {}

    void IDEFramePresenter::onInitialized(IDEFrame *view, DialogManager *dialogView, MenuPanel *menuView) {
        assert(view);

        this->view = view;
        this->dialogView = dialogView;
        this->menuView = menuView;

        const auto menu = MenuData{MenuData::menuBar({
            MenuData::menu("&File", {
                MenuData::action([this] () { this->onFileNew(); }, "&New", ActionId::DocumentNew, {Modifier::Ctrl, Key::N}),
                MenuData::separator(),
                MenuData::action([this] () { this->onFileOpen(); }, "&Open ...", ActionId::ShowOpenDocumentDialog, {Modifier::Ctrl, Key::O}),
                MenuData::action([this] () { this->onFileOpenFolder(); }, "Open &Folder ...", ActionId::ShowOpenFolderDialog, {Modifier::CtrlShift, Key::O}),
                MenuData::separator(),
                MenuData::action([this] () { this->onFileSave(); }, "&Save", ActionId::DocumentSave, {Modifier::Ctrl, Key::S}),
                MenuData::action([this] () { this->onFileSaveAs(); }, "Sa&ve As ...", ActionId::ShowSaveDocumentDialog),
                MenuData::action([this] () { this->onFileSaveAll(); }, "Save &All", ActionId::DocumentSaveAll),
                MenuData::separator(),
                MenuData::action([this] () { this->onFileClose(); }, "&Close", ActionId::DocumentClose),
                MenuData::separator(),
                MenuData::action([this] () { this->onFileExit(); }, "&Exit", ActionId::AppExit)
            }),
            MenuData::menu("&Edit", {
                MenuData::action([this] () { this->onEditUndo(); }, "&Undo", {}, {Modifier::Ctrl, Key::Z}),
                MenuData::action([this] () { this->onEditRedo(); }, "&Redo", {}, {Modifier::CtrlShift, Key::Z}),
                MenuData::separator(),
                MenuData::action([this] () { this->onEditCut(); }, "&Cut", {}, {Modifier::Ctrl, Key::X}),
                MenuData::action([this] () { this->onEditCopy(); }, "C&opy", {}, {Modifier::Ctrl, Key::C}),
                MenuData::action([this] () { this->onEditPaste(); }, "&Paste", {}, {Modifier::Ctrl, Key::V}),
                MenuData::separator(),
                MenuData::action([] () {}, "Find ..."),
                MenuData::action([] () {}, "Replace ..."),
            }),
            MenuData::menu("&View", {
                MenuData::action([this] () { this->onViewFolderBrowser(); }, "&Folder Browser")
            }),
            MenuData::menu("&Tools", {
                MenuData::action([this] () { this->onToolsFileSearch(); }, "File &Search ...", {}, {Modifier::CtrlShift, Key::P})
            }),
            MenuData::menu("&Build", {
                MenuData::action([] () {}, "&Clean"),
                MenuData::action([] () {}, "&Execute")
            }),
            MenuData::menu("&Help", {
                MenuData::action([] () {}, "&About", ActionId::ShowAboutDialog, {Key::F1})
            }),
        })};

        this->menuView->setupMenuBar(menu);
    }

    void IDEFramePresenter::onFileNew() {
        documentManagerPresenter->onNewDocument();
    }

    void IDEFramePresenter::onFileOpen() {
        auto fileDialog = FileDialogData{};
        fileDialog.title = "Open File";
        fileDialog.filters = model->getFileFilters();
        fileDialog.type = FileDialogType::OpenFile;

        if (auto filePath = dialogView->showFileDialog(fileDialog)) {
            documentManagerPresenter->onOpenDocument(filePath.value().string());
        }
    }

    void IDEFramePresenter::onFileOpen(const std::string &filePath) {
        documentManagerPresenter->onOpenDocument(filePath);
    }

    void IDEFramePresenter::onFileOpenFolder() {
        auto folderDialog = FolderDialogData{};
        folderDialog.title = "Open Folder";

        if (auto folderPath = dialogView->showFolderDialog(folderDialog)) {
            folderBrowserPresenter->onDisplayFolder(folderPath.value().string());
            model->setWorkspaceFolder(folderPath.value().string());
        }
    }

    void IDEFramePresenter::onToolsFileSearch() {
        if (auto workspaceFolder = model->getWorkspaceFolder()) {
            auto fileSearchDialog = FileSearchDialogData{};

            fileSearchDialog.title = "File Search";
            // fileSearchDialog.defaultPath = workspaceFolder.get();

            if (auto filePath = dialogView->showFileSearchDialog(fileSearchDialog)) {
                documentManagerPresenter->onOpenDocument(filePath.value().string());
            }
        }
    }

    void IDEFramePresenter::onFileSave() {
        documentManagerPresenter->onSaveDocument();
    }

    void IDEFramePresenter::onFileSaveAs() {
        documentManagerPresenter->onSaveAsDocument();
    }

    void IDEFramePresenter::onFileSaveAll() {
        documentManagerPresenter->onSaveAllDocuments();
    }

    void IDEFramePresenter::onEditUndo() {
        // TODO: Add implementation
    }

    void IDEFramePresenter::onEditRedo() {
        // TODO: Add implementation
    }

    void IDEFramePresenter::onEditCut() {
        // TODO: Add implementation
    }

    void IDEFramePresenter::onEditCopy() {
        // TODO: Add implementation
    }

    void IDEFramePresenter::onEditPaste() {
        // TODO: Add implementation
    }

    void IDEFramePresenter::onFileClose() {
        documentManagerPresenter->onCloseCurrentDocument();
    }

    void IDEFramePresenter::onFileExit() {
        view->close();
    }

    bool IDEFramePresenter::onCloseRequested() {
        assert(this);
        assert(view->getDialogManager());

        auto messageDialog = MessageDialogData {};
        messageDialog.title = "Xenoide";
        messageDialog.message = "Exit?";
        messageDialog.icon = DialogIcon::Question;
        messageDialog.buttons = DialogButton::YesNo;

        const DialogButton button = dialogView->showMessageDialog(messageDialog);

        return button == DialogButton::Yes;
    }

    void IDEFramePresenter::onViewFolderBrowser() {
        view->showPanel(IDEFrame::FOLDER_BROWSER);
    }

    void IDEFramePresenter::openFolder(const std::string &fullPath) {
        view->getFolderBrowser()->displayFolder(fullPath);
    }

    DocumentManagerPresenter*  IDEFramePresenter::getDocumentManagerPresenter() {
        return documentManagerPresenter.get();
    }

    FolderBrowserPresenter* IDEFramePresenter::getFolderBrowserPresenter() {
        return folderBrowserPresenter.get();
    }
}
