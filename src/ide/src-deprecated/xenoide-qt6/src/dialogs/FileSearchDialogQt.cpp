
#include "FileSearchDialogQt.h"

#include <iostream>
#include <QVBoxLayout>
#include "FileSearchDialogItemWidgetQt.h"

#include <xenoide/ui/FileSearchDialogPresenter.h>

namespace xenoide {
    FileSearchDialogQt::FileSearchDialogQt(QWidget *parent, FileSearchDialogPresenter *presenter) : QDialog(parent), FileSearchDialog(presenter) {
        this->filePatternLineEdit = new QLineEdit();
        this->filesListWidget = new QListWidget();
        this->okCancelButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

        QObject::connect(this->okCancelButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        QObject::connect(this->okCancelButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

        auto layout = new QVBoxLayout();

        layout->addWidget(this->filePatternLineEdit);
        layout->addWidget(this->filesListWidget);
        layout->addWidget(this->okCancelButtonBox);

        this->setLayout(layout);

        QObject::connect(this->filePatternLineEdit, &QLineEdit::textEdited, [this](const QString &text) {
            this->presenter->onFilenameFilterRequested(text.toStdString());
        });

        QObject::connect(this->filesListWidget, &QListView::doubleClicked, [this]() {
            this->accept();
        });

        this->presenter->onInitialized(this);
    }

    FileSearchDialogQt::~FileSearchDialogQt() {}

    QString FileSearchDialogQt::selectedFile() const {
        QString selectedFile;

        if (auto item = this->filesListWidget->currentItem()) {
            if (auto widget = dynamic_cast<FileSearchDialogItemWidgetQt*>(filesListWidget->itemWidget(item))) {
                selectedFile = widget->getFilePath();
            }
        }

        std::cout << selectedFile.toStdString() << std::endl;

        return selectedFile;
    }

    void FileSearchDialogQt::displayFileList(const std::vector<FileSearchDialog::FileViewData> &files) {
        for (const FileSearchDialog::FileViewData &fileViewData : files) {
            auto fileItem = new QListWidgetItem();
            this->filesListWidget->addItem(fileItem);

            auto fileItemWidget = new FileSearchDialogItemWidgetQt(
                nullptr, 
                fileViewData.fileTitle.c_str(), 
                fileViewData.fileFolder.c_str(),
                fileViewData.filePath.c_str()
            );

            this->filesListWidget->setItemWidget(fileItem, fileItemWidget);
        }
    }

    void FileSearchDialogQt::clearFileList() {
        this->filesListWidget->clear();
    }

    void FileSearchDialogQt::hide() {
        
    }
}
