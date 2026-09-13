
#include "DocumentMdiSubWindowQt.h"

#include <cassert>
#include <iostream>

#include <QEvent>
#include <QGridLayout>
#include <QAction>
#include <QMenu>
#include <xenoide/ui/DocumentPresenter.h>

namespace xenoide {
    DocumentMdiSubWindowQt::DocumentMdiSubWindowQt(Document2 *document) : dialogManager(this), document(document) {
        scintilla = new ScintillaEdit(this);

        this->setWidget(scintilla);
        this->setupContextMenu();
    }

    void DocumentMdiSubWindowQt::closeEvent(QCloseEvent *evt) {
        emit closeRequested(this, evt);
    }

    void DocumentMdiSubWindowQt::setupLayout() {
        QGridLayout *layout = new QGridLayout(this);
        layout->addWidget(scintilla);
        this->setLayout(layout);
    }

    void DocumentMdiSubWindowQt::setupContextMenu() {
        systemMenu()->clear();

        auto closeAction = new QAction{"Close", this};
        systemMenu()->addAction(closeAction);
        connect(closeAction, &QAction::triggered, [this]() {
            // this->presenter->onCloseDocument(editor);
        });

        auto closeAllButThisAction = new QAction("Close all but this", this);
        systemMenu()->addAction(closeAllButThisAction);
        connect(closeAllButThisAction, &QAction::triggered, [this]() {
            // this->presenter->onCloseOtherDocuments(editor);
        });

        auto closeAllAction = new QAction("Close all", this);
        systemMenu()->addAction(closeAllAction);
        connect(closeAllAction, &QAction::triggered, [this]() {
            // this->presenter->onCloseAllDocuments();
        });

        auto closeToTheRightAction = new QAction("Close to the right", this);
        systemMenu()->addAction(closeToTheRightAction);
        connect(closeToTheRightAction, &QAction::triggered, [this]() {
            // this->presenter->onCloseDocumentsToTheRight(editor);
        });
    }
}
