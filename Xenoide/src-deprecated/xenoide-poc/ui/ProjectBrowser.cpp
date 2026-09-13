
#include "ProjectBrowser.hpp"

#include <iostream>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <iostream>

namespace felide {
    ProjectBrowser::ProjectBrowser (QWidget *parent) 
        : QWidget(parent) {

        m_treeView = new QTreeView(this);

        this->setLayout(new QVBoxLayout(this));
        this->layout()->addWidget(m_treeView);

        // instance file system model
        m_fileSystemModel = new QFileSystemModel(this);
        m_fileSystemModel->setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);

        // bind view and model together
        m_treeView->setModel(m_fileSystemModel);
        m_treeView->setVisible(false);

        // connect signal handlers
        QObject::connect(m_treeView, &QTreeView::doubleClicked, [this](const QModelIndex index) {
            QString path = this->m_fileSystemModel->fileInfo(index).absoluteFilePath();

            this->projectItemOpenRequest(path);
        });
    }

    ProjectBrowser::~ProjectBrowser() {}

    void ProjectBrowser::setProjectFolder(const QString &projectFolder) {
        m_treeView->setVisible(true);

        m_projectFolder = projectFolder;
        m_fileSystemModel->setRootPath(projectFolder);

        QModelIndex index = m_fileSystemModel->index(projectFolder);

        m_treeView->setRootIndex(index);
    }

    QString ProjectBrowser::projectFolder() const {
        return m_projectFolder;
    }
}
