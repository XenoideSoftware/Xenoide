
#include <xenoide/ui/FindReplaceDialog.h>

#include <xenoide/ui/FindReplaceDialogPresenter.h>
#include <xenoide/ui/StringUtil.h>
#include <xenoide/ui/Document.h>

#include <iostream>

namespace xenoide {
    void FindReplaceDialogPresenter::attachView(FindReplaceDialog *view, Document *documentView) {
        this->view = view;
        this->documentView = documentView;

        view->attachPresenter(this);
    }

    void FindReplaceDialogPresenter::handleEditFindReplace() {
        FindReplaceDialog::ViewData viewData;

        viewData.title = "Find/Replace in Document ...";
        viewData.findWhat = this->findWhat;
        viewData.matchCase = this->matchCase;
        viewData.matchWholeWorld = this->matchWholeWord;
        viewData.replaceWith = this->replaceWith;

        if (this->scope == Scope::CurrentDocument) {
            viewData.scope = FindReplaceDialog::CurrentDocument;
        } else if (this->scope == Scope::Selection) {
            viewData.scope = FindReplaceDialog::Selection;
        }

        view->show(viewData);
    }


    void FindReplaceDialogPresenter::handleFindWhatTextBox_Change(const std::string &value) {
        findWhat = value;
    }


    void FindReplaceDialogPresenter::handleReplaceWithCheckBox_Click(const bool checked) {
        replaceInsteadOfFind = checked;

        view->toggleReplaceControls(checked);
    }


    void FindReplaceDialogPresenter::handleReplaceWithTextBox_Change(const std::string &value) {
        replaceWith = value;
    }


    void FindReplaceDialogPresenter::handleMatchCaseCheckBox_Click(const bool checked) {
        matchCase = checked;
    }


    void FindReplaceDialogPresenter::handleMatchWholeWordCheckBox_Click(const bool checked) {
        matchWholeWord = checked;
    }


    void FindReplaceDialogPresenter::handleSelectionScopeOptionBox_Click() {
        scope = Selection;
    }


    void FindReplaceDialogPresenter::handleCurrentDocumentScopeOptionBox_Click() {
        scope = CurrentDocument;
    }


    void FindReplaceDialogPresenter::handleFindNextButton_Click() {
        const size_t currentOffset = documentView->getSelection().end;

        if (! findNext(currentOffset)) {
            findNext(0);
        }
    }


    bool FindReplaceDialogPresenter::findNext(const size_t currentOffset) {
        const std::string content = this->documentView->getContent();
        const auto flags = FIND_FLAGS((matchCase ? FF_MATCH_CASE : FF_DEFAULT) | (matchWholeWord ? FF_MATCH_WHOLE_WORD : FF_DEFAULT));

        const std::size_t offset = find(content.c_str(), currentOffset, findWhat.c_str(), flags);

        if (offset == std::string::npos) {
            return false;
        } 

        const std::size_t length = findWhat.size();
        documentView->setSelection({(int)offset, (int)(offset + length)});
        
        return true;
    }


    void FindReplaceDialogPresenter::handleReplaceNextButton_Click() {
        // TODO: Add implementation
        std::cout << "asdad" << std::endl;
    }


    void FindReplaceDialogPresenter::handleReplaceAllButton_Click() {
        // TODO: Add implementation
        std::cout << "asdad" << std::endl;
    }


    void FindReplaceDialogPresenter::handleCloseButton_Click() {
        // TODO: Add implementation
        std::cout << "asdad" << std::endl;
    }

}
