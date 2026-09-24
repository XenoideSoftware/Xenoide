#include "CodeEditorPresenter.h"

#include "CodeEditor.h"
#include "xenoide/core/FileService.h"

namespace xenoide {
    CodeEditorPresenter::CodeEditorPresenter(const gsl_lite::not_null<FileService *> &newFileService) : fileService(newFileService) {
    }

    void CodeEditorPresenter::onInitialized(const gsl_lite::not_null<CodeEditor *> &newView) {
        view = newView;
        view->setTitle(computeTitle());
    }

    void CodeEditorPresenter::onNew() {
        filePath.clear();
        modified = false;
        view->setContent("");
        view->setTitle(computeTitle());
    }

    void CodeEditorPresenter::onContentChanged() {
        modified = true;

        view->setTitle(computeTitle());
    }

    void CodeEditorPresenter::onSave() {
        if (!hasFilePath()) {
            return;
        }

        const std::string content = view->getContent();
        fileService->save(filePath.string(), content);

        modified = false;

        view->setTitle(computeTitle());
    }

    void CodeEditorPresenter::onSaveAs(const std::filesystem::path &newFilePath) {
        filePath = newFilePath;

        onSave();
    }

    void CodeEditorPresenter::openFile(const std::filesystem::path &path) {
        filePath = path;

        const std::string content = fileService->load(filePath.string());

        view->setContent(content);

        modified = false;

        view->setTitle(computeTitle());
    }

    bool CodeEditorPresenter::isModified() const {
        return modified;
    }

    bool CodeEditorPresenter::hasFilePath() const {
        return !filePath.empty();
    }

    std::filesystem::path CodeEditorPresenter::getFilePath() const {
        return filePath;
    }

    std::string CodeEditorPresenter::computeTitle() const {
        std::string name;

        if (hasFilePath()) {
            name = filePath.filename().string();
        } else {
            // TODO: Migrate this raw string to a parametrized constant when considering internationalization
            name = "Untitled";
        }

        if (modified) {
            return "[*]" + name;
        }

        return name;
    }
} // namespace xenoide
