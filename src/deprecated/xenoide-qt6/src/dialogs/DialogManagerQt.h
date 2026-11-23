
#ifndef __XENOIDE_UI_DIALOGMANAGERVIEWIMPL_HPP__
#define __XENOIDE_UI_DIALOGMANAGERVIEWIMPL_HPP__

#include <xenoide/ui/DialogManager.h>
#include <QWidget>

namespace xenoide {
    class DialogManagerQt : public DialogManager {
    public:
        explicit DialogManagerQt(QWidget *parent);
        virtual ~DialogManagerQt();

        virtual DialogButton showMessageDialog(const MessageDialogData &data) const override;
        virtual std::optional<std::filesystem::path> showFileDialog(const FileDialogData &data) const override;
        virtual std::optional<std::filesystem::path> showFolderDialog(const FolderDialogData &data) override;
        virtual std::optional<std::string> showInputDialog(const InputDialogData &data) const override;
        virtual std::optional<std::filesystem::path> showFileSearchDialog(const FileSearchDialogData &data) const override;

    private:
        QWidget *m_parent = nullptr;
    };
} // namespace xenoide

#endif
