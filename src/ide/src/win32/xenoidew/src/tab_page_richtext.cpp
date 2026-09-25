#include "tab_page_richtext.h"
#include <tchar.h>
#include <winlambxe/utils/sys_brush.h>

TabPageRichText::TabPageRichText() {
    this->setup.wndClassEx.lpszClassName = _T("WL_RICHTEXT_PAGE");
    this->setup.wndClassEx.hbrBackground = wlx::sys_color_brush(COLOR_WINDOW);

    this->on_message(WM_CREATE, [this](wl::params) -> LRESULT {
        RECT rc{};
        GetClientRect(this->hwnd(), &rc);
        this->_edit.create(this->hwnd(), 1, {0, 0}, {rc.right, rc.bottom});
        this->_edit.set_font(_T("Consolas"), 11);
        this->_edit.set_text(_T("Welcome to ModernWinApp\r\n")
                             _T("==============================\r\n\r\n")
                             _T("This is a RichEdit control.\r\n\r\n")
                             _T("Wrapper features (wlx::richedit):\r\n")
                             _T("  - set_text / get_text\r\n")
                             _T("  - append_text (EM_REPLACESEL)\r\n")
                             _T("  - set_font via CHARFORMAT2 / SCF_ALL\r\n")
                             _T("  - set_readonly\r\n")
                             _T("  - Full ES_MULTILINE with vertical scroll\r\n"));
        return 0;
    });

    this->on_message(WM_SIZE, [this](wl::params p) -> LRESULT {
        const int cx = LOWORD(p.lParam);
        const int cy = HIWORD(p.lParam);
        if (this->_edit.hwnd()) {
            SetWindowPos(this->_edit.hwnd(), nullptr, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        return 0;
    });
}
