
#include <xenoide/ui/DocumentPresenter.h>

#include <xenoide/core/OS.h>
#include <xenoide/core/FileService.h>
#include <xenoide/ui/DialogManager.h>
#include <xenoide/ui/Document.h>
#include <xenoide/ui/DocumentModel.h>

#include <filesystem>

namespace xenoide {
    DocumentPresenter::DocumentPresenter(DocumentModel *model) {
        this->model = model;
    }

    DocumentPresenter::~DocumentPresenter() {
    }

    void DocumentPresenter::onInitialized(Document *view, DialogManager *dialogView) {
        this->view = view;
        this->dialogView = dialogView;

        if (model->hasFilePath()) {
            auto fileService = FileService();
            auto content = fileService.load(model->getFilePath());

            model->setContent(content);
        }

        const std::string title = this->computeTitle(model);
        view->setTitle(title);
        view->setContent(model->getContent());
        view->setConfig(DocumentConfig::Default());
    }

    void DocumentPresenter::onContentChanged() {
        model->modify();

        const std::string title = this->computeTitle(model);
        view->setTitle(title);
    }

    DocumentPresenter::UserResponse DocumentPresenter::onSave() {
        // TODO: Put FileFilter getters from a PresenterService

        if (!model->hasFilePath()) {
            auto fileDialog = FileDialogData{};
            fileDialog.title = "Save File";
            fileDialog.type = FileDialogType::SaveFile;
            fileDialog.defaultPath = this->computeFileTitle(model);
            fileDialog.filters = {FileFilter{"All Files", {"*"}}};

            if (auto filePath = dialogView->showFileDialog(fileDialog)) {
                // model->setFilePath(filePath.get().string());
            } else {
                return DocumentPresenter::UserResponse::Cancel;
            }
        }

        auto fileService = FileService();

        model->setContent(view->getContent());

        const std::string fileName = model->getFilePath();
        const std::string content = model->getContent();

        fileService.save(fileName, content);

        model->setFilePath(fileName);
        model->setModifiedFlag(false);

        const std::string title = this->computeTitle(model);

        view->setTitle(title);

        return DocumentPresenter::UserResponse::Accept;
    }

    DocumentPresenter::UserResponse DocumentPresenter::onSaveAs() {
        // TODO: Setup a PresenterService to support the UI
        auto fileDialog = FileDialogData{};

        fileDialog.title = "Save File";
        fileDialog.type = FileDialogType::SaveFile;
        fileDialog.filters = {FileFilter{"All Files", {"*"}}};

        if (auto filePath = dialogView->showFileDialog(fileDialog)) {
            // model->setFilePath(filePath.get().string());

            this->onSave();

            return DocumentPresenter::UserResponse::Accept;
        }

        return DocumentPresenter::UserResponse::Cancel;
    }

    DocumentPresenter::UserResponse DocumentPresenter::onCloseRequested() {
        if (!model->getModifiedFlag()) {
            return DocumentPresenter::UserResponse::Accept;
        }

        auto messageDialog = MessageDialogData{};
        messageDialog.title = "Xenoide";
        messageDialog.message = "The file has unsaved changes. Save them before closing?";
        messageDialog.icon = DialogIcon::Question;
        messageDialog.buttons = DialogButton::YesNoCancel;

        auto dialogResult = dialogView->showMessageDialog(messageDialog);

        switch (dialogResult) {
        case DialogButton::Cancel:
            return DocumentPresenter::UserResponse::Cancel;

        case DialogButton::Yes:
            return this->onSave();

        case DialogButton::No:
            return DocumentPresenter::UserResponse::Accept;
        }

        return DocumentPresenter::UserResponse::Cancel;
    }

    void DocumentPresenter::onTitleChanged() {
        const std::string title = this->computeTitle(model);
        view->setTitle(title);
    }

    std::string DocumentPresenter::computeFileTitle(DocumentModel *model) const {
        if (model->hasFilePath()) {
            return std::filesystem::path(model->getFilePath()).filename().string();
        }

        return "Untitled " + std::to_string(model->getTag());
    }

    std::string DocumentPresenter::computeTitle(DocumentModel *model) const {
        const std::string prefix = (model->getModifiedFlag() ? "[*]" : "");
        const std::string fileTitle = this->computeFileTitle(model);

        return prefix + fileTitle;
    }

    bool DocumentPresenter::hasFilePath(const std::filesystem::path &filePath) const {
        return std::filesystem::path(model->getFilePath()) == filePath;
    }

    Document *DocumentPresenter::getView() const {
        return view;
    }

    DocumentModel *DocumentPresenter::getModel() const {
        return model;
    }
} // namespace xenoide
