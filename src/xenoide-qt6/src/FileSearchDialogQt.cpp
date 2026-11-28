
#include "FileSearchDialogQt.h"

#include <iostream>
#include <QVBoxLayout>
#include "FileSearchDialogItemWidgetQt.h"

namespace xenoide {
    FileSearchDialogQt::FileSearchDialogQt(QWidget *parent) : QDialog(parent) {
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
    }

    FileSearchDialogQt::~FileSearchDialogQt() {
    }

    QString FileSearchDialogQt::selectedFile() const {
        QString selectedFile;

        if (auto item = this->filesListWidget->currentItem()) {
            if (auto widget = dynamic_cast<FileSearchDialogItemWidgetQt *>(filesListWidget->itemWidget(item))) {
                selectedFile = widget->getFilePath();
            }
        }

        std::cout << selectedFile.toStdString() << std::endl;

        return selectedFile;
    }

} // namespace xenoide
