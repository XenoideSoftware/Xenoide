#pragma once

#include "winlamb/dialog_modal.h"
#include "winlamb/textbox.h"
#include "winlamb/button.h"
#include "winlamb/label.h"
#include "winlamb/checkbox.h"
#include "winlamb/resizer.h"

class FindDialog : public wl::dialog_modal {
public:
    FindDialog();

    int show(HWND hParent);

    static INT_PTR CALLBACK dialog_proc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

    const wl::tstring &find_text() const noexcept {
        return _findText;
    }
    const wl::tstring &replace_text() const noexcept {
        return _replaceText;
    }
    bool match_case() const noexcept {
        return _matchCase;
    }
    bool whole_word() const noexcept {
        return _wholeWord;
    }

private:
    // Controls
    wl::label _lblSearch;
    wl::textbox _txtSearch;
    wl::label _lblReplace;
    wl::textbox _txtReplace;
    wl::checkbox _chkCase;
    wl::checkbox _chkWord;
    wl::button _btnFindNext;
    wl::button _btnReplace;
    wl::button _btnReplaceAll;
    wl::button _btnCancel;

    // Fields to retrieve after dialog closes
    wl::tstring _findText;
    wl::tstring _replaceText;
    bool _matchCase = false;
    bool _wholeWord = false;
    int _result = 1010; // IDC_BTN_CANCEL

    wl::resizer _resz;

    // Control IDs
    static constexpr int IDC_LBL_SEARCH = 1001;
    static constexpr int IDC_TXT_SEARCH = 1002;
    static constexpr int IDC_LBL_REPLACE = 1003;
    static constexpr int IDC_TXT_REPLACE = 1004;
    static constexpr int IDC_CHK_CASE = 1005;
    static constexpr int IDC_CHK_WORD = 1006;
    static constexpr int IDC_BTN_FINDNEXT = 1007;
    static constexpr int IDC_BTN_REPLACE = 1008;
    static constexpr int IDC_BTN_REPLACEALL = 1009;
    static constexpr int IDC_BTN_CANCEL = 1010;
};
