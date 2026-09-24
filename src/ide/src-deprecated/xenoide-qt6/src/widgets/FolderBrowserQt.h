
#ifndef __XENOIDE_UI_QT5_FOLDERBROWSER_HPP__
#define __XENOIDE_UI_QT5_FOLDERBROWSER_HPP__

#include <QWidget>
#include <QTreeView>
#include <QFileSystemModel>

#include <xenoide/ui/FolderBrowser.h>

namespace xenoide {
    class DialogManagerQt;
    class DialogManager;

    class FolderBrowserQt : public QWidget {
        Q_OBJECT

    public:
        explicit FolderBrowserQt(QWidget *parent);

        void setRootFolder(const QString &rootFolder);

        QString getRootFolder() const;

        QString getSelectedPath() const;

    signals:
        void pathSelected(const QString &path);

        void pathActivated(const QString &path);

        void pathContextMenuRequested(const QString &path, const QPoint &pos);

        void pathMoveRequested(const QString &targetPath);

    private:
        QString rootFolder;
        QTreeView *treeView = nullptr;
        QFileSystemModel *model = nullptr;
    };
} // namespace xenoide

#endif
