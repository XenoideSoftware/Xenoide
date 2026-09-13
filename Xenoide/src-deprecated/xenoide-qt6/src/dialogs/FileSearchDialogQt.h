
#ifndef __XENOIDE_UI_QT5_FILESEARCHDIALOGQT_HPP__
#define __XENOIDE_UI_QT5_FILESEARCHDIALOGQT_HPP__

#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QDialogButtonBox>
#include <xenoide/ui/FileSearchDialog.h>
#include "DialogManagerQt.h"

namespace xenoide {
    class FileSearchDialogQt : public QDialog, public FileSearchDialog {
        Q_OBJECT

    public:
        explicit FileSearchDialogQt(QWidget *parent, FileSearchDialogPresenter *presenter);

        virtual ~FileSearchDialogQt();

        QString selectedFile() const;

    public:
        virtual void displayFileList(const std::vector<FileViewData> &files) override;

        virtual void hide() override;

        virtual void clearFileList() override;

    private:
        QLineEdit *filePatternLineEdit = nullptr;
        QListWidget *filesListWidget = nullptr;
        QDialogButtonBox *okCancelButtonBox = nullptr;
    };
}

#endif
