
#ifndef __XENOIDE_UI_QT5_FILESEARCHDIALOGQT_HPP__
#define __XENOIDE_UI_QT5_FILESEARCHDIALOGQT_HPP__

#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QDialogButtonBox>


namespace xenoide {
    class FileSearchDialogQt : public QDialog {
        Q_OBJECT

    public:
        explicit FileSearchDialogQt(QWidget *parent);

        virtual ~FileSearchDialogQt();

        QString selectedFile() const;

    private:
        QLineEdit *filePatternLineEdit = nullptr;
        QListWidget *filesListWidget = nullptr;
        QDialogButtonBox *okCancelButtonBox = nullptr;
    };
} // namespace xenoide

#endif
