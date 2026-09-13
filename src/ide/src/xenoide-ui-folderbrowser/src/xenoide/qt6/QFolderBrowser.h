#pragma once

#include <QWidget>

#include <string>
#include <unordered_map>
#include <gsl-lite/gsl-lite.hpp>

#include <xenoide/FolderBrowser.h>

class QTreeWidget;
class QTreeWidgetItem;

namespace xenoide {
    class FolderBrowserPresenter;
}

namespace xenoide::qt6 {
    class QFolderBrowser : public QWidget, public xenoide::FolderBrowser {
        Q_OBJECT

    public:
        explicit QFolderBrowser(const gsl_lite::not_null<FolderBrowserPresenter*> &presenter, QWidget *parent = nullptr);

        // ------------------------------------------------------------------
        // FolderBrowser interface
        // ------------------------------------------------------------------
        void clearItems() override;
        void addRootItem(int itemId, const std::string &name, const std::string &path) override;
        void addChildItem(int parentId, int childId, const std::string &name, const std::string &path, bool isFolder) override;
        void expandItem(int itemId) override;

    signals:
        /**
         * Emitted when the user expands a tree node.
         * The host should forward this to FolderBrowserPresenter::onItemExpanded.
         */
        void itemExpanded(int itemId);

        /**
         * Emitted when the user double-clicks a file entry.
         * The host can connect this to open the file in the code editor.
         */
        void fileActivated(const QString &filePath);

    private:
        QTreeWidgetItem *findItem(int itemId) const;

        QTreeWidget *treeWidget = nullptr;
        std::unordered_map<int, QTreeWidgetItem *> items;
        gsl_lite::not_null<FolderBrowserPresenter*> presenter;
    };
} // namespace xenoide::qt6
