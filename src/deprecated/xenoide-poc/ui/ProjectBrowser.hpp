
#pragma once

#ifndef __felide_projectbrowser_hpp__
#define __felide_projectbrowser_hpp__

#include <QWidget>
#include <QDockWidget>
#include <QTreeView>
#include <QFileSystemModel>

namespace felide {
    class ProjectBrowser : public QWidget {
        Q_OBJECT

    public:
        ProjectBrowser(QWidget *parent);

        ~ProjectBrowser();

        void setProjectFolder(const QString &projectFolder);

        QString projectFolder() const;

        Q_PROPERTY(QString projectFolder WRITE setProjectFolder READ projectFolder NOTIFY projectFolderChanged)

    signals:
        void projectFolderChanged(const QString &projectFolder);

        void projectItemOpenRequest(const QString &projectItemPath);

    private:
        QString m_projectFolder;
        QTreeView *m_treeView = nullptr;
        QFileSystemModel *m_fileSystemModel = nullptr;
    };
} // namespace felide

#endif
