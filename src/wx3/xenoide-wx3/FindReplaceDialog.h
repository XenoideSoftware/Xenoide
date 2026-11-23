#pragma once

#include <wx/wx.h>

namespace xenoide {

    class FindReplaceDialog : public wxDialog {
    public:
        FindReplaceDialog(wxWindow *parent);
        virtual ~FindReplaceDialog();

        // Getters for UI state
        wxString getFindString() const;
        wxString getReplaceString() const;
        bool isCaseSensitive() const;
        bool isWholeWord() const;
        bool isInSelection() const;

    private:
        wxTextCtrl *mFindText = nullptr;
        wxTextCtrl *mReplaceText = nullptr;
        wxCheckBox *mCaseCheckBox = nullptr;
        wxCheckBox *mWholeWordCheckBox = nullptr;
        wxCheckBox *mInSelectionCheckBox = nullptr;
    };

} // namespace xenoide
