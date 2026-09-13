
#pragma once

#include <QWidget>
#include <QGroupBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>

#include <xenoide/ui/FindReplaceDialog.h>

namespace xenoide {
    class DocumentMdiSubWindowQt;
    class FindReplaceDialogQt : public QWidget, public FindReplaceDialog {
        Q_OBJECT

    public:
        explicit FindReplaceDialogQt(QWidget *parent, FindReplaceDialogPresenter *presenter, DocumentMdiSubWindowQt *document);

        virtual ~FindReplaceDialogQt();

    private:
        void setupSignals();

        void setupLayout();

    public:
        void attachPresenter(FindReplaceDialogPresenter *presenter) override;

        void show(const ViewData &viewData) override;

        void hide() override;

        void toggleReplaceControls(const bool status) override;

    private:
        QGroupBox *findReplaceGroupBox = nullptr;
        QLabel *findWhatLabel = nullptr;
        QLineEdit *findWhatEdit = nullptr;
        QCheckBox *replaceWithCheckBox = nullptr;
        QLineEdit *replaceWithEdit = nullptr;

        QGroupBox *matchGroupBox = nullptr;
        QCheckBox *matchCaseCheckBox = nullptr;
        QCheckBox *matchWholeWordCheckBox = nullptr;

        QGroupBox *scopeGroupBox = nullptr;
        QRadioButton *scopeSelectionRadio = nullptr;
        QRadioButton *scopeCurrentDocumentRadio = nullptr;

        QPushButton *findNextButton = nullptr;
        QPushButton *replaceNextButton = nullptr;
        QPushButton *replaceAllButton = nullptr;
        QPushButton *closeButton = nullptr;
    };
}
