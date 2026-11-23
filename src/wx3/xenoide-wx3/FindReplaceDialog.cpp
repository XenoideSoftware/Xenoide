#include "FindReplaceDialog.h"
#include <wx/xrc/xmlres.h>

namespace xenoide {

    FindReplaceDialog::FindReplaceDialog(wxWindow *parent) {
        wxXmlResource::Get()->LoadDialog(this, parent, "FindReplaceDialog");

        mFindText = XRCCTRL(*this, "txtFind", wxTextCtrl);
        mReplaceText = XRCCTRL(*this, "txtReplace", wxTextCtrl);
        mCaseCheckBox = XRCCTRL(*this, "chkMatchCase", wxCheckBox);
        mWholeWordCheckBox = XRCCTRL(*this, "chkWholeWord", wxCheckBox);
        mInSelectionCheckBox = XRCCTRL(*this, "chkInSelection", wxCheckBox);

        GetSizer()->Fit(this);
        GetSizer()->SetSizeHints(this);
        Centre();
    }

    FindReplaceDialog::~FindReplaceDialog() {
    }

    wxString FindReplaceDialog::getFindString() const {
        return mFindText->GetValue();
    }

    wxString FindReplaceDialog::getReplaceString() const {
        return mReplaceText->GetValue();
    }

    bool FindReplaceDialog::isCaseSensitive() const {
        return mCaseCheckBox->GetValue();
    }

    bool FindReplaceDialog::isWholeWord() const {
        return mWholeWordCheckBox->GetValue();
    }

    bool FindReplaceDialog::isInSelection() const {
        return mInSelectionCheckBox->GetValue();
    }

} // namespace xenoide
