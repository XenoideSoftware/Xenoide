
#pragma once

#include <map>
#include <optional>
#include <vector>
#include <QWidget>
#include <QMdiArea>
#include <QMdiSubWindow>

#include "dialogs/DialogManagerQt.h"
#include "DocumentMdiSubWindowQt.h"

namespace xenoide {
    class DocumentMdiSubWindowQt;
    class DocumentManagerMdiQt : public QWidget {
        Q_OBJECT

    public:
        explicit DocumentManagerMdiQt(QWidget *parent);

        virtual ~DocumentManagerMdiQt() = default;

        DocumentMdiSubWindowQt *appendDocumentWindow();

        void setCurrentDocument(DocumentMdiSubWindowQt *documentWindow);

        DocumentMdiSubWindowQt *getCurrentDocument() const;

    private:
        [[nodiscard]]
        std::optional<int> getDocumentIndex(const QPoint &pos) const;

        QMenu *contextMenu;

        QAction *closeAction;
        QAction *closeAllButThisAction;
        QAction *closeAllAction;
        QAction *closeToTheRightAction;

        QMdiArea *mdiArea = nullptr;
        DialogManagerQt dialogManager;
    };
} // namespace xenoide
