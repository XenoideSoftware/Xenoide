
#pragma once

#include <QMdiSubWindow>
#include <QCloseEvent>

#include <ScintillaEdit.h>

namespace xenoide {
    class DocumentMdiSubWindowQt : public QMdiSubWindow {
        Q_OBJECT

    public:
        explicit DocumentMdiSubWindowQt(Document2 *document);

        ~DocumentMdiSubWindowQt() final = default;

        void closeEvent(QCloseEvent *evt) override;

    signals:
        void closeRequested(DocumentMdiSubWindowQt *documentWindow, QCloseEvent *evt);

        void contextMenuRequested(DocumentMdiSubWindowQt *documentWindow);

    private:
        void setupLayout();

        void setupContextMenu();

        ScintillaEdit *scintilla = nullptr;
    };
} // namespace xenoide
