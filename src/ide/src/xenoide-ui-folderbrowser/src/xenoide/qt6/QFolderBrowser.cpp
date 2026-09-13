#include "QFolderBrowser.h"

#include "xenoide/FolderBrowserPresenter.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace {
    // Roles stored per tree item.
    constexpr int kRoleItemId = Qt::UserRole;       // int  – presenter-assigned ID
    constexpr int kRoleFilePath = Qt::UserRole + 1; // QString – full filesystem path
} // anonymous namespace

namespace xenoide::qt6 {
    QFolderBrowser::QFolderBrowser(const gsl_lite::not_null<FolderBrowserPresenter*> &presenter, QWidget *parent) : QWidget(parent), presenter(presenter) {
        treeWidget = new QTreeWidget(this);
        treeWidget->setHeaderHidden(true);
        treeWidget->setAnimated(true);
        treeWidget->setUniformRowHeights(true);

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(treeWidget);

        // Forward QTreeWidget expansion to our typed signal.
        connect(treeWidget, &QTreeWidget::itemExpanded, this, [this](QTreeWidgetItem *item) { emit itemExpanded(item->data(0, kRoleItemId).toInt()); });

        // Emit fileActivated only when the activated item is a file (no children).
        connect(treeWidget, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem *item, int /*column*/) {
            if (item->childCount() == 0) {
                const QString path = item->data(0, kRoleFilePath).toString();
                if (!path.isEmpty())
                    emit fileActivated(path);
            }
        });

        presenter->onInitialized(gsl_lite::make_not_null(this));
    }

    // ------------------------------------------------------------------
    // FolderBrowser interface
    // ------------------------------------------------------------------

    void QFolderBrowser::clearItems() {
        treeWidget->clear();
        items.clear();
    }

    void QFolderBrowser::addRootItem(int itemId, const std::string &name, const std::string &path) {
        auto *item = new QTreeWidgetItem(QStringList{QString::fromStdString(name)});
        item->setData(0, kRoleItemId, itemId);
        item->setData(0, kRoleFilePath, QString::fromStdString(path));
        treeWidget->addTopLevelItem(item);
        items[itemId] = item;
    }

    void QFolderBrowser::addChildItem(int parentId, int childId, const std::string &name, const std::string &path, bool isFolder) {
        QTreeWidgetItem *parent = findItem(parentId);
        if (!parent)
            return;

        auto *child = new QTreeWidgetItem(QStringList{QString::fromStdString(name)});
        child->setData(0, kRoleItemId, childId);
        child->setData(0, kRoleFilePath, QString::fromStdString(path));

        // Folder items get a placeholder child so Qt shows the expand arrow
        // before the user triggers lazy loading.
        if (isFolder) {
            child->addChild(new QTreeWidgetItem());
        }

        parent->addChild(child);
        items[childId] = child;
    }

    void QFolderBrowser::expandItem(int itemId) {
        if (QTreeWidgetItem *item = findItem(itemId)) {
            treeWidget->expandItem(item);
        }
    }

    // ------------------------------------------------------------------
    // Private helpers
    // ------------------------------------------------------------------

    QTreeWidgetItem *QFolderBrowser::findItem(int itemId) const {
        const auto it = items.find(itemId);
        return it != items.end() ? it->second : nullptr;
    }
} // namespace xenoide::qt6
