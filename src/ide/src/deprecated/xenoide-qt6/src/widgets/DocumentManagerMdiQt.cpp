
#include "DocumentManagerMdiQt.h"

#include <iostream>
#include <QGridLayout>
#include <QAction>
#include <QMenu>
#include <QMdiSubWindow>
#include <QTextEdit>

#include <xenoide/ui/Document.h>
#include "DocumentMdiSubWindowQt.h"
#include <xenoide/ui/DocumentPresenter.h>

namespace xenoide {
    DocumentManagerMdiQt::DocumentManagerMdiQt(QWidget *parent) : QWidget(parent), dialogManager(this) {
        closeAction = new QAction{"Close", this};
        closeAllButThisAction = new QAction("Close all but this", this);
        closeAllAction = new QAction("Close all", this);
        closeToTheRightAction = new QAction("Close to the right", this);

        connect(closeAction, &QAction::triggered, [this]() {});
        connect(closeAllButThisAction, &QAction::triggered, [this]() {});
        connect(closeAllAction, &QAction::triggered, [this]() {});
        connect(closeToTheRightAction, &QAction::triggered, [this]() {});

        contextMenu = new QMenu("Context Menu", this);
        contextMenu->addAction(closeAction);
        contextMenu->addAction(closeAllButThisAction);
        contextMenu->addAction(closeAllAction);
        contextMenu->addAction(closeToTheRightAction);

        mdiArea = new QMdiArea(this);
        mdiArea->setViewMode(QMdiArea::TabbedView);
        mdiArea->setTabsClosable(true);
        mdiArea->setTabsMovable(true);
        mdiArea->setDocumentMode(true);

        mdiArea->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(mdiArea, &QMdiArea::customContextMenuRequested, [this](const QPoint &pos) {
            if (this->getDocumentIndex(pos)) {
                contextMenu->exec(this->mapToGlobal(pos));
            }
        });

        auto layout = new QGridLayout(this);
        layout->addWidget(mdiArea);
        this->setLayout(layout);
    }

    std::optional<int> DocumentManagerMdiQt::getDocumentIndex(const QPoint &pos) const {
        QList<QMdiSubWindow *> subWindows = mdiArea->subWindowList();

        for (int i = 0; i < subWindows.count(); i++) {
            if (const QRect rect = subWindows[i]->rect(); rect.contains(pos)) {
                return i;
            }
        }

        return {};
    }

    DocumentMdiSubWindowQt *DocumentManagerMdiQt::appendDocumentWindow() {
        assert(mdiArea);

        // TODO: This document-tab initialization logic is private to the CustomMdiSubWindow. Consider refactor it later
        auto documentSubWindowQt = new DocumentMdiSubWindowQt(nullptr);

        documentSubWindowQt->setAttribute(Qt::WA_DeleteOnClose, true);

        // handle the tab close request
        this->connect(documentSubWindowQt, &DocumentMdiSubWindowQt::closeRequested, [=](DocumentMdiSubWindowQt *subWindow, QCloseEvent *evt) {
            // TODO: Check if we can close
            const bool userAccepted = true;

            if (userAccepted) {
                evt->accept();
            } else {
                evt->ignore();
            }
        });

        mdiArea->addSubWindow(documentSubWindowQt);

        documentSubWindowQt->setWindowTitle("Document!");
        documentSubWindowQt->show();

        return documentSubWindowQt;
    }

    void DocumentManagerMdiQt::setCurrentDocument(DocumentMdiSubWindowQt *documentWindow) {
        assert(mdiArea);
        assert(documentWindow);

        mdiArea->setActiveSubWindow(documentWindow);
    }

    DocumentMdiSubWindowQt *DocumentManagerMdiQt::getCurrentDocument() const {
        assert(mdiArea);

        return dynamic_cast<DocumentMdiSubWindowQt *>(mdiArea->activeSubWindow());
    }
} // namespace xenoide
