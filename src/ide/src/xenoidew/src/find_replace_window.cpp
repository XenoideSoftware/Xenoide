
#include "find_replace_window.h"
#include "winlamb/font.h"
#include <tchar.h>

namespace {
    static constexpr int IDC_LBL_SEARCH     = 1001;
    static constexpr int IDC_TXT_SEARCH     = 1002;
    static constexpr int IDC_LBL_REPLACE    = 1003;
    static constexpr int IDC_TXT_REPLACE    = 1004;
    static constexpr int IDC_CHK_CASE       = 1005;
    static constexpr int IDC_CHK_WORD       = 1006;
    static constexpr int IDC_BTN_FINDNEXT   = 1007;
    static constexpr int IDC_BTN_REPLACE    = 1008;
    static constexpr int IDC_BTN_REPLACEALL = 1009;
    static constexpr int IDC_BTN_CANCEL     = 1010;
}


FindReplaceWindow::FindReplaceWindow() {
	setup.wndClassEx.lpszClassName = _T("XENOIDE_FIND_REPLACE_WINDOW");

	on_message(WM_CREATE, [this](wl::wm::size p) -> LRESULT {
		
        // setup initial layout
        RECT rcClient = {};
        GetClientRect(hwnd(), &rcClient);
        int const width = rcClient.right - rcClient.left;

        int const leftPadding = 16;
        int const rightPadding = 16;

        int const textHeight = 50;
        int const searchTop = 10;

        this->_txtSearch.create(hwnd(), IDC_TXT_SEARCH, wl::textbox::type::NORMAL, { leftPadding, 15 }, width - rightPadding - leftPadding, textHeight);
        this->_txtReplace.create(hwnd(), IDC_TXT_REPLACE, wl::textbox::type::NORMAL, { leftPadding, 80 }, width - rightPadding - leftPadding, textHeight);

        this->_chkCase.create(hwnd(), IDC_CHK_CASE, _T("Match case"), { 10, 75 }, { 100, 20 });
        this->_chkWord.create(hwnd(), IDC_CHK_WORD, _T("Whole word"), { 120, 75 }, { 100, 20 });

        this->_btnFindNext.create(hwnd(), IDC_BTN_FINDNEXT, _T("Find Next"), { 10, 110 }, { 80, 25 });
        this->_btnReplace.create(hwnd(), IDC_BTN_REPLACE, _T("Replace"), { 95, 110 }, { 80, 25 });
        this->_btnReplaceAll.create(hwnd(), IDC_BTN_REPLACEALL, _T("Replace All"), { 180, 110 }, { 80, 25 });
        this->_btnCancel.create(hwnd(), IDC_BTN_CANCEL, _T("Cancel"), { 265, 110 }, { 80, 25 });

        wl::font::util::set_ui_on_children(hwnd());

        this->_resz
            .add(this->_txtSearch,  wl::resizer::go::RESIZE, wl::resizer::go::NOTHING)
            .add(this->_txtReplace, wl::resizer::go::RESIZE, wl::resizer::go::NOTHING)
            .add(this->_lblSearch,  wl::resizer::go::NOTHING, wl::resizer::go::NOTHING)
            .add(this->_lblReplace, wl::resizer::go::NOTHING, wl::resizer::go::NOTHING)
            .add(this->_chkCase,    wl::resizer::go::NOTHING, wl::resizer::go::REPOS)
            .add(this->_chkWord,    wl::resizer::go::NOTHING, wl::resizer::go::REPOS)
            .add({this->_btnFindNext, this->_btnReplace,
                this->_btnReplaceAll, this->_btnCancel},
                wl::resizer::go::REPOS, wl::resizer::go::REPOS);

		return 0;
	});


    on_message(WM_SIZE, [this](wl::wm::create p) -> LRESULT {

        return 0;
        });

}
