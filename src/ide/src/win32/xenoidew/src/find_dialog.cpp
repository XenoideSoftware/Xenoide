#include "find_dialog.h"
#include "winlamb/font.h"
#include "winlamb/resizer.h"

namespace {
    struct DlgTemplate {
        DLGTEMPLATE t;
        WORD menu = 0;
        WORD windowClass = 0;
        WORD title = 0;
    };

    // Control IDs
    constexpr int IDC_LBL_SEARCH = 1001;
    constexpr int IDC_TXT_SEARCH = 1002;
    constexpr int IDC_LBL_REPLACE = 1003;
    constexpr int IDC_TXT_REPLACE = 1004;
    constexpr int IDC_CHK_CASE = 1005;
    constexpr int IDC_CHK_WORD = 1006;
    constexpr int IDC_BTN_FINDNEXT = 1007;
    constexpr int IDC_BTN_REPLACE = 1008;
    constexpr int IDC_BTN_REPLACEALL = 1009;
    constexpr int IDC_BTN_CANCEL = 1010;
} // namespace

FindDialog::FindDialog() = default;

int FindDialog::show(HWND hParent) {
    DlgTemplate templ{};
    // WS_THICKFRAME lets the user actually grab an edge/corner to resize;
    // without it the dialog is fixed-size regardless of the resizer.
    templ.t.style = DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME;
    templ.t.dwExtendedStyle = 0;
    templ.t.cdit = 0;
    templ.t.x = 0;
    templ.t.y = 0;
    templ.t.cx = 10;
    templ.t.cy = 10;

    INT_PTR ret = DialogBoxIndirectParam(
#if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0500
        hParent ? reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE)) : GetModuleHandle(nullptr),
#else
        hParent ? reinterpret_cast<HINSTANCE>(GetWindowLong(hParent, GWL_HINSTANCE)) : GetModuleHandle(nullptr),
#endif
        reinterpret_cast<LPDLGTEMPLATE>(&templ),
        hParent,
        dialog_proc,
        reinterpret_cast<LPARAM>(this)
    );

    return static_cast<int>(ret);
}

SIZE computeTextSize(HWND hWnd, const wl::tstring &text) {
    HDC const hDC = GetDC(hWnd);

    auto const hFont = reinterpret_cast<HFONT>(SendMessage(hWnd, WM_GETFONT, 0, 0));
    auto const hOld = static_cast<HFONT>(SelectObject(hDC, hFont));

    RECT rect = {};
    DrawText(hDC, text.c_str(), -1, &rect, DT_CALCRECT | DT_SINGLELINE);

    SelectObject(hDC, hOld);
    ReleaseDC(hWnd, hDC);

    return {rect.right - rect.left, rect.top - rect.bottom};
}

INT_PTR CALLBACK FindDialog::dialog_proc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp) {
    FindDialog *pSelf = nullptr;
    if (msg == WM_INITDIALOG) {
        pSelf = reinterpret_cast<FindDialog *>(lp);
#if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0500
        SetWindowLongPtr(hDlg, DWLP_USER, reinterpret_cast<LONG_PTR>(pSelf));
#else
        SetWindowLong(hDlg, DWL_USER, reinterpret_cast<LONG>(pSelf));
#endif

        SetWindowText(hDlg, _T("Find and Replace"));

        // Setup a reasonable pixel-based size (360x150 client area)
        RECT rc{0, 0, 360, 150};
        AdjustWindowRectEx(&rc, WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | DS_MODALFRAME, FALSE, 0);
        SetWindowPos(hDlg, nullptr, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE);

        // Center on parent
        HWND hParent = GetParent(hDlg);
        if (hParent) {
            RECT rcDlg{}, rcParent{};
            GetWindowRect(hDlg, &rcDlg);
            GetWindowRect(hParent, &rcParent);
            SetWindowPos(
                hDlg,
                nullptr,
                rcParent.left + (rcParent.right - rcParent.left) / 2 - (rcDlg.right - rcDlg.left) / 2,
                rcParent.top + (rcParent.bottom - rcParent.top) / 2 - (rcDlg.bottom - rcDlg.top) / 2,
                0,
                0,
                SWP_NOZORDER | SWP_NOSIZE
            );
        }

        // setup initial layout
        RECT rcClient = {};
        GetClientRect(hDlg, &rcClient);
        int const width = rcClient.right - rcClient.left;

        int const leftPadding = 16;
        int const rightPadding = 16;

        int const textHeight = 50;
        int const searchTop = 10;

        pSelf->_txtSearch.create(hDlg, IDC_TXT_SEARCH, wl::textbox::type::NORMAL, {leftPadding, 15}, width - rightPadding - leftPadding, textHeight);
        pSelf->_txtReplace.create(hDlg, IDC_TXT_REPLACE, wl::textbox::type::NORMAL, {leftPadding, 80}, width - rightPadding - leftPadding, textHeight);

        pSelf->_chkCase.create(hDlg, IDC_CHK_CASE, _T("Match case"), {10, 75}, {100, 20});
        pSelf->_chkWord.create(hDlg, IDC_CHK_WORD, _T("Whole word"), {120, 75}, {100, 20});

        pSelf->_btnFindNext.create(hDlg, IDC_BTN_FINDNEXT, _T("Find Next"), {10, 110}, {80, 25});
        pSelf->_btnReplace.create(hDlg, IDC_BTN_REPLACE, _T("Replace"), {95, 110}, {80, 25});
        pSelf->_btnReplaceAll.create(hDlg, IDC_BTN_REPLACEALL, _T("Replace All"), {180, 110}, {80, 25});
        pSelf->_btnCancel.create(hDlg, IDC_BTN_CANCEL, _T("Cancel"), {265, 110}, {80, 25});

        // Apply system UI font on all dynamically created controls
        wl::font::util::set_ui_on_children(hDlg);

        // --- Resizer setup ---
        // Search/Replace textboxes stretch horizontally with the dialog.
        // Labels and checkboxes stay put (fixed top area).
        // The buttons, anchored bottom, stay fixed-size but reposition
        // as a block relative to the growing right/bottom edges.
        pSelf->_resz.add(pSelf->_txtSearch, wl::resizer::go::RESIZE, wl::resizer::go::NOTHING)
            .add(pSelf->_txtReplace, wl::resizer::go::RESIZE, wl::resizer::go::NOTHING)
            .add(pSelf->_lblSearch, wl::resizer::go::NOTHING, wl::resizer::go::NOTHING)
            .add(pSelf->_lblReplace, wl::resizer::go::NOTHING, wl::resizer::go::NOTHING)
            .add(pSelf->_chkCase, wl::resizer::go::NOTHING, wl::resizer::go::REPOS)
            .add(pSelf->_chkWord, wl::resizer::go::NOTHING, wl::resizer::go::REPOS)
            .add({pSelf->_btnFindNext, pSelf->_btnReplace, pSelf->_btnReplaceAll, pSelf->_btnCancel}, wl::resizer::go::REPOS, wl::resizer::go::REPOS);

        SetFocus(pSelf->_txtSearch.hwnd());
        return FALSE; // Return FALSE because we set the focus manually
    } else {
#if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0500
        pSelf = reinterpret_cast<FindDialog *>(GetWindowLongPtr(hDlg, DWLP_USER));
#else
        pSelf = reinterpret_cast<FindDialog *>(GetWindowLong(hDlg, DWL_USER));
#endif
    }

    if (pSelf) {
        if (msg == WM_SIZE) {
            wl::params p;

            p.lParam = lp;
            p.wParam = wp;
            p.message = msg;

            pSelf->_resz.adjust(p);
            return 0;
        }
        if (msg == WM_CLOSE) {
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        if (msg == WM_COMMAND) {
            WORD ctrlId = LOWORD(wp);
            if (ctrlId == IDC_BTN_CANCEL || ctrlId == IDCANCEL) {
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }
            if (ctrlId == IDC_BTN_FINDNEXT || ctrlId == IDC_BTN_REPLACE || ctrlId == IDC_BTN_REPLACEALL) {
                pSelf->_findText = pSelf->_txtSearch.get_text();
                pSelf->_replaceText = pSelf->_txtReplace.get_text();
                pSelf->_matchCase = pSelf->_chkCase.is_checked();
                pSelf->_wholeWord = pSelf->_chkWord.is_checked();

                EndDialog(hDlg, ctrlId);
                return TRUE;
            }
        }
    }

    return FALSE;
}