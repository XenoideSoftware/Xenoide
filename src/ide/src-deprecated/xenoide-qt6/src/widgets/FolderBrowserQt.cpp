
#include "FolderBrowserQt.h"

#include <QMimeData>
#include <QVBoxLayout>

#include <filesystem>

namespace xenoide {
    namespace fs = std::filesystem;

    // TODO: Refactor file handling logic into another layer
    class FolderBrowserQtTreeModel : public QFileSystemModel {
    public:
        FolderBrowserQtTreeModel(FolderBrowserQt *parent) : QFileSystemModel(parent), folderBrowser(parent) {
        }

        Qt::ItemFlags flags(const QModelIndex &index) const override {
            if (!index.isValid()) {
                return {};
            }

            const auto path = fs::path(this->filePath(index).toStdString());
            const auto flags = Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);

            if (fs::is_directory(path)) {
                return flags | Qt::ItemIsDropEnabled;
            }

            return flags;
        }

        Qt::DropActions supportedDropActions() const override {
            return Qt::DropActions(Qt::CopyAction | Qt::MoveAction);
        }

        bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override {
            if (action == Qt::MoveAction) {
                const QString targetPath = this->fileInfo(parent).absoluteFilePath();
                folderBrowser->pathMoveRequested(targetPath);
            }

            return QFileSystemModel::dropMimeData(data, action, row, column, parent);
        }

    private:
        FolderBrowserQt *folderBrowser = nullptr;
    };
} // namespace xenoide

namespace xenoide {
    FolderBrowserQt::FolderBrowserQt(QWidget *parent) : QWidget(parent) {
        treeView = new QTreeView(this);

        this->setLayout(new QVBoxLayout(this));
        this->layout()->addWidget(treeView);

        // instance file system model
        model = new FolderBrowserQtTreeModel(this);
        model->setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);

        // enable treeview support
        treeView->setDragDropMode(QAbstractItemView::InternalMove);
        // m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
        treeView->setDragEnabled(true);
        treeView->setAcceptDrops(true);
        // m_treeView->setDropIndicatorShown(true);

        // bind view and model together
        treeView->setModel(model);
        treeView->setVisible(false);
        treeView->setHeaderHidden(true);
        treeView->setContextMenuPolicy(Qt::CustomContextMenu);

        for (int i = 1; i < model->columnCount(); ++i) {
            treeView->hideColumn(i);
        }

        // connect signal handlers
        connect(treeView->selectionModel(), &QItemSelectionModel::currentChanged, [this](const QModelIndex &current, const QModelIndex &previous) {
            pathSelected(model->filePath(current));
        });

        connect(treeView, &QTreeView::doubleClicked, [this](const QModelIndex &index) { pathActivated(model->filePath(index)); });

        connect(treeView, &QTreeView::customContextMenuRequested, [this](const QPoint &pos) { pathContextMenuRequested(model->filePath(treeView->currentIndex()), pos); });
    }

    void FolderBrowserQt::setRootFolder(const QString &projectFolder) {
        treeView->setVisible(true);

        rootFolder = projectFolder;
        model->setRootPath(projectFolder);

        QModelIndex index = model->index(projectFolder);
        treeView->setRootIndex(index);
    }

    QString FolderBrowserQt::getRootFolder() const {
        return rootFolder;
    }

    QString FolderBrowserQt::getSelectedPath() const {
        return model->filePath(treeView->currentIndex());
    }
} // namespace xenoide
