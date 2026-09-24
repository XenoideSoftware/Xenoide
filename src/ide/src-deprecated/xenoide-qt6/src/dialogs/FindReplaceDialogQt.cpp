
#include "FindReplaceDialogQt.h"

namespace xenoide {
    template <typename TWidget> void addChildToParent(QWidget *parent, TWidget **widget, const QRect &geometry, const QString &title) {
        *widget = new TWidget(title, parent);
        (*widget)->setGeometry(geometry);
    }

    FindReplaceDialogQt::FindReplaceDialogQt(QWidget *parent, FindReplaceDialogPresenter *presenter, DocumentMdiSubWindowQt *document) : QWidget(parent) {
        addChildToParent(this, &findReplaceGroupBox, {10, 10, 270, 80}, "Find/Replace parameters");
        addChildToParent(this, &findWhatLabel, {30, 30, 100, 20}, "Find what");
        addChildToParent(this, &findWhatEdit, {150, 30, 100, 20}, "");
        addChildToParent(this, &replaceWithCheckBox, {30, 55, 100, 20}, "Replace with");
        addChildToParent(this, &replaceWithEdit, {150, 55, 100, 20}, "");

        addChildToParent(this, &matchGroupBox, {10, 100, 270, 80}, "Match options");
        addChildToParent(this, &matchCaseCheckBox, {30, 125, 125, 20}, "Match case");
        addChildToParent(this, &matchWholeWordCheckBox, {30, 145, 125, 20}, "Match whole word");

        addChildToParent(this, &scopeGroupBox, {10, 190, 270, 80}, "Scope");
        addChildToParent(this, &scopeSelectionRadio, {30, 210, 125, 20}, "Selection");
        addChildToParent(this, &scopeCurrentDocumentRadio, {30, 230, 125, 20}, "Current Document");

        addChildToParent(this, &findNextButton, {10, 280, 120, 30}, "Find Next");
        addChildToParent(this, &closeButton, {140, 280, 120, 30}, "Close");
        addChildToParent(this, &replaceNextButton, {10, 320, 120, 30}, "Replace Next");
        addChildToParent(this, &replaceAllButton, {140, 320, 120, 30}, "Replace All");

        this->setupLayout();
        this->setupSignals();
    }

    FindReplaceDialogQt::~FindReplaceDialogQt() {
    }

    void FindReplaceDialogQt::setupSignals() {
        // TODO: Add implementation
    }

    void FindReplaceDialogQt::setupLayout() {
    }

    void FindReplaceDialogQt::attachPresenter(FindReplaceDialogPresenter *presenter) {
    }

    void FindReplaceDialogQt::show(const ViewData &viewData) {
    }

    void FindReplaceDialogQt::hide() {
    }

    void FindReplaceDialogQt::toggleReplaceControls(const bool status) {
    }
} // namespace xenoide
