
#include "main_window_controller.h"

#include <cmath>
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <numeric>

#include "xenoide/core/StringUtil.h"

MainWindowController::MainWindowController() = default;

MainWindowController::MainWindowController(MainWindowView *view, const SciEditor &editor) : view(view) {
    model = std::make_unique<MainWindowModel>(editor);
}

void MainWindowController::onNewFileCommand() {
    model->new_();
}

void MainWindowController::onOpenFileCommand() {
    std::string const filter = model->getFileFilter();
    std::optional<std::string> const selectedPath = view->showFileDialog(ShowFileDialog::Open, {filter});

    if (!selectedPath) {
        return;
    }

    model->load(selectedPath);
}

void MainWindowController::onSaveFileCommand() {
    if (model->canSave()) {
        model->save({});
    } else {
        onSaveAsFileCommand();
    }
}

void MainWindowController::onSaveAsFileCommand() {
    namespace fs = std::filesystem;

    ShowFileDialogOptions dialogOptions;
    dialogOptions.filter = model->getFileFilter();
    dialogOptions.defaultFile = fs::path(model->getFilePath().value_or("Untitled.cpp")).filename().string();

    std::optional<std::string> const selectedPath = view->showFileDialog(ShowFileDialog::Save, dialogOptions);

    if (!selectedPath) {
        return;
    }

    model->save(selectedPath);
}

void MainWindowController::onEditorUndoCommand() {
    model->getEditor().undo();
}

void MainWindowController::onEditorRedoCommand() {
    model->getEditor().redo();
}

void MainWindowController::onEditorCutCommand() {
    model->getEditor().cut();
}

void MainWindowController::onEditorCopyCommand() {
    model->getEditor().copy();
}

void MainWindowController::onEditorPasteCommand() {
    model->getEditor().paste();
}

void MainWindowController::onEditorModified() {
    model->getEditor().syncMarginLineNumber(0);

    MainWindowNotification notification;
    notification.modifiedFlagChanged = true;
    model->notify(notification);
}

void MainWindowController::onEditorCharAdded(SCNotification const &notification) {
    // marks each new line, white characters and tabs insertions as undo point
    if (notification.ch == '\n' || notification.ch == '\t' || notification.ch == ' ') {
        model->getEditor().markUndoPoint();
    }
}

void MainWindowController::onClose() {
    bool const isModified = model->isModified();

    if (isModified) {
        ShowMessageDialogOptions options;
        options.title = "Xenoide";
        options.prompt = "File is modified. Do you want to save it?";

        ShowMessageDialogButton const selectedButton = view->showMessageDialog(options);

        switch (selectedButton) {
        case ShowMessageDialogButton::Yes:
            onSaveFileCommand();
            view->postQuitMessage();
            return;

        case ShowMessageDialogButton::No:
            view->postQuitMessage();
            return;

        case ShowMessageDialogButton::Cancel:
        default:
            return;
        }
    } else {
        view->postQuitMessage();
    }
}
