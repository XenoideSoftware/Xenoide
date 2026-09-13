
#include "FileSearchDialogItemWidgetQt.h"

#include <QLabel>
#include <QHBoxLayout>

namespace xenoide {
    FileSearchDialogItemWidgetQt::FileSearchDialogItemWidgetQt(QWidget *parent, const QString &fileTitle, const QString &fileFolder, const QString &filePath) : QWidget(parent) {
        auto fileTitleLabel = new QLabel(fileTitle);
        auto filePathLabel = new QLabel(fileFolder);

        auto layout = new QHBoxLayout();

        layout->addWidget(fileTitleLabel);
        layout->addWidget(filePathLabel);

        this->setLayout(layout);

        this->filePath = filePath;
    }
}
