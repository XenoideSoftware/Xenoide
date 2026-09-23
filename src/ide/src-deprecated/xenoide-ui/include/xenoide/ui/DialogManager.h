
#ifndef __XENOIDE_UI_DIALOGMANAGER_HPP__
#define __XENOIDE_UI_DIALOGMANAGER_HPP__

#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <xenoide/ui/FileFilter.h>
#include <xenoide/core/Predef.h>

namespace xenoide {
    enum class FileDialogType { OpenFile, SaveFile };

    struct FileDialogData {
        std::string title;
        FileDialogType type;
        std::vector<FileFilter> filters;
        std::filesystem::path defaultPath;
    };

    struct FolderDialogData {
        std::string title;
        std::filesystem::path defaultPath;
    };

    enum class DialogIcon { Information, Warning, Error, Question };

    enum class DialogButton { Ok = 1, Cancel = 2, Yes = 4, No = 8, OkCancel = Ok | Cancel, YesNoCancel = Yes | No | Cancel, YesNo = Yes | No };

    inline DialogButton operator|(const DialogButton f1, const DialogButton f2) {
        return static_cast<DialogButton>(static_cast<int>(f1) | static_cast<int>(f2));
    }

    inline bool operator&(const DialogButton f1, const DialogButton f2) {
        if (f1 & f2) {
            return true;
        }

        return false;
    }

    struct MessageDialogData {
        std::string title;
        std::string message;
        DialogIcon icon;
        DialogButton buttons;
    };

    struct InputDialogData {
        std::string title;
        std::string label;
        std::string defaultText;
    };

    struct FileSearchDialogData {
        std::string title;
        std::filesystem::path defaultPath;
    };

    class DialogManager {
    public:
        virtual ~DialogManager();
        virtual DialogButton showMessageDialog(const MessageDialogData &data) const = 0;
        virtual std::optional<std::string> showInputDialog(const InputDialogData &data) const = 0;
        virtual std::optional<std::filesystem::path> showFileDialog(const FileDialogData &data) const = 0;
        virtual std::optional<std::filesystem::path> showFolderDialog(const FolderDialogData &data) = 0;
        virtual std::optional<std::filesystem::path> showFileSearchDialog(const FileSearchDialogData &data) const = 0;
    };
} // namespace xenoide

#endif
