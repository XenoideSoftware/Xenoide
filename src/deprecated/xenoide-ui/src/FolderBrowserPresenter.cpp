

#include "xenoide/ui/ActionBus.h"

#include <xenoide/ui/FolderBrowserPresenter.h>

#include <xenoide/ui/FolderBrowser.h>
#include <xenoide/ui/FolderBrowserModel.h>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <system_error>

#include <xenoide/ui/DialogManager.h>
#include <xenoide/ui/Menu.h>
#include <xenoide/ui/IDEFrame.h>

namespace xenoide {
    static std::string describePathKind(const std::filesystem::path &path) {
        if (std::filesystem::is_directory(path)) {
            return "directory";
        } else {
            return "file";
        }
    }

    FolderBrowserPresenter::FolderBrowserPresenter(ActionBus *actionBus, FolderBrowserModel *model) {
        this->actionBus = actionBus;
        this->model = model;
    }

    FolderBrowserPresenter::~FolderBrowserPresenter() {
    }

    void FolderBrowserPresenter::onInitialized(FolderBrowser *folderBrowser, DialogManager *dialogManager) {
        this->view = folderBrowser;
        this->dialogView = dialogManager;
    }

    void FolderBrowserPresenter::onBrowseFolder() {
        if (auto folderPath = dialogView->showFolderDialog({"Open Folder", ""})) {
            // model->setCurrentFolderPath(folderPath.get());
            // view->displayFolder(folderPath.get().string());
        }
    }

    void FolderBrowserPresenter::onCreateFile() {
        const auto selectedPathOptional = view->getSelectedPath();
        if (!selectedPathOptional) {
            return;
        }

        const auto selectedPath = std::filesystem::path(*selectedPathOptional);

        // ask for the new filename
        const auto newFileName = this->askValidPath("Xenoide", "Please, enter the new file name", "Previous name was invalid. Enter the new file name", "Newfile");

        if (!newFileName) {
            return;
        }

        // construct the new file path
        std::filesystem::path filePath;

        if (std::filesystem::is_directory(selectedPath)) {
            filePath = selectedPath / newFileName.value();
        } else {
            filePath = selectedPath.parent_path() / newFileName.value();
        }

        // TODO: Replace logic with the FileService class
        // perform a "touch" function
        std::ofstream os;
        os.open(filePath.string().c_str(), std::ios_base::out);
        os.close();
    }

    void FolderBrowserPresenter::onCreateFolder() {
        //
        const auto selectedPathOptional = view->getSelectedPath();
        if (!selectedPathOptional) {
            return;
        }

        const auto selectedPath = std::filesystem::path(*selectedPathOptional);

        // ask the new folder name
        const auto newFolderName = this->askValidPath("Xenoide", "Please, enter the new folder name", "Previous name was invalid. Enter the new folder name", "Newfolder");

        if (!newFolderName) {
            return;
        }

        // construct folder path
        std::filesystem::path folderPath;

        if (std::filesystem::is_directory(selectedPath)) {
            folderPath = selectedPath / newFolderName.value();
        } else {
            folderPath = selectedPath.parent_path() / newFolderName.value();
        }

        // create the directory on his final location
        namespace fs = std::filesystem;
        std::error_code errorCode;

        fs::create_directory(folderPath, errorCode);

        // TODO: Notify to the view the change in the filesystem (?)
    }

    void FolderBrowserPresenter::onOpenSelectedFile() {
        // determine the currently selected path
        if (const auto selectedPath = view->getSelectedPath()) {
            const auto filePath = std::filesystem::path(*selectedPath);

            if (!std::filesystem::is_directory(filePath)) {
                // actionBus->sendAction(ActionId::DocumentOpen);
            }
        }
    }

    void FolderBrowserPresenter::onMoveSelectedPath(const std::string &targetFolder) {
        // TODO: Add directory check to the targetFolder variable

        namespace fs = std::filesystem;

        // determine the currently selected path
        const auto selectedPathOptional = view->getSelectedPath();
        if (!selectedPathOptional) {
            return;
        }

        const auto selectedPath = fs::path(*selectedPathOptional);

        // compute destination path
        const auto destinationPath = fs::path(targetFolder) / selectedPath.filename();

        if (selectedPath == destinationPath) {
            return;
        }

        // existence check!
        if (fs::exists(destinationPath)) {
            auto messageDialog = MessageDialogData{};
            messageDialog.title = "Xenoide";
            messageDialog.message = "File/Directory already exists. Replace it?";
            messageDialog.icon = DialogIcon::Warning;
            messageDialog.buttons = DialogButton::YesNo;

            const auto selectedButton = dialogView->showMessageDialog(messageDialog);

            if (selectedButton == DialogButton::No) {
                return;
            }
        } else if (std::filesystem::is_directory(selectedPath)) {
            // prompt the user confirmation
            auto messageDialog = MessageDialogData{};
            messageDialog.title = "Xenoide";
            messageDialog.message = "Move the directory \"" + selectedPath.filename().string() + "\"?";
            messageDialog.icon = DialogIcon::Warning;
            messageDialog.buttons = DialogButton::OkCancel;

            if (auto selectedButton = dialogView->showMessageDialog(messageDialog) == DialogButton::Cancel) {
                return;
            }
        }

        fs::rename(selectedPath, destinationPath);
    }

    void FolderBrowserPresenter::onRenameSelectedPath() {
        namespace fs = std::filesystem;

        // determine the currently selected path
        const auto selectedPathOptional = view->getSelectedPath();
        if (!selectedPathOptional) {
            return;
        }

        const auto selectedPath = fs::path(*selectedPathOptional);
        const auto pathKind = describePathKind(selectedPath);
        const auto prompt = "Please, enter a new name for the \"" + selectedPath.filename().string() + "\" " + pathKind;
        const auto prefix = "Invalid " + pathKind + " name. ";

        // prompt the user for a new path
        std::optional<std::string> newFilenameOptional = this->askValidPath("Xenoide", prompt, prefix + prompt, selectedPath.filename().string());

        if (!newFilenameOptional) {
            return;
        }

        // compute new path
        const auto newFilename = fs::path(*newFilenameOptional);
        const auto newPath = selectedPath.parent_path() / newFilename;

        if (std::filesystem::exists(newPath)) {
            auto messageDialog = MessageDialogData{};
            messageDialog.title = "Xenoide";
            messageDialog.message = "Another file already exists.";
            messageDialog.icon = DialogIcon::Error;
            messageDialog.buttons = DialogButton::Ok;

            dialogView->showMessageDialog(messageDialog);
            return;
        }

        // do the rename
        std::filesystem::rename(selectedPath, newPath);

        // TODO: Notify to the view the change in the filesystem (?)
    }

    void FolderBrowserPresenter::onDeleteSelectedPath() {
        namespace fs = std::filesystem;

        // determine the currently selected path
        const auto selectedPathOptional = view->getSelectedPath();
        if (!selectedPathOptional) {
            return;
        }

        const auto selectedPath = fs::path(*selectedPathOptional);

        // prompt the user confirmation
        auto messageDialog = MessageDialogData{};
        messageDialog.title = "Xenoide";
        messageDialog.message = "Delete the \"" + selectedPath.filename().string() + "\" " + describePathKind(selectedPath) + "?";
        messageDialog.icon = DialogIcon::Warning;
        messageDialog.buttons = DialogButton::OkCancel;

        if (auto selectedButton = dialogView->showMessageDialog(messageDialog) == DialogButton::Cancel) {
            return;
        }

        // do the delete
        if (std::filesystem::is_directory(selectedPath)) {
            std::filesystem::remove_all(selectedPath);
        } else {
            std::filesystem::remove(selectedPath);
        }

        // TODO: Notify to the view the change in the filesystem (?)
    }

    std::optional<std::string>
    FolderBrowserPresenter::askValidPath(const std::string &title, const std::string &prompt, const std::string &promptForInvalidInput, const std::string &defaultValue) {
        int attemped = 0;

        std::optional<std::string> validNamePath = {};

        while (true) {
            const std::string finalPrompt = (!attemped ? prompt : promptForInvalidInput);

            auto inputDialog = InputDialogData{};
            inputDialog.title = title;
            inputDialog.label = finalPrompt;
            inputDialog.defaultText = defaultValue;

            validNamePath = dialogView->showInputDialog(inputDialog);
            if (!validNamePath /*|| std::filesystem::native(*validNamePath)*/) {
                break;
            }

            ++attemped;
        }

        return validNamePath;
    }

    void FolderBrowserPresenter::onDisplayFolder(const std::filesystem::path &folderPath) {
        model->setCurrentFolderPath(folderPath.string());
        view->displayFolder(folderPath.string());
    }

    void FolderBrowserPresenter::onContextMenuRequested(const Point &point) {
        const auto menu = MenuData::menu(
            "Context Menu",
            {MenuData::action([this]() { this->onOpenSelectedFile(); }, "Open"),
             MenuData::separator(),
             MenuData::action([this]() { this->onCreateFile(); }, "Create File"),
             MenuData::action([this]() { this->onCreateFolder(); }, "Create Folder"),
             MenuData::separator(),
             MenuData::action([this]() { this->onRenameSelectedPath(); }, "Rename"),
             MenuData::action([this]() { this->onDeleteSelectedPath(); }, "Delete")}
        );

        view->displayContextualMenu(point, menu);
    }
} // namespace xenoide
