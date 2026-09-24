#include "winlambxe/webview.h"
#include "winlambxe/utils/sys_brush.h"

wlx::webview::webview() {
    this->setup.wndClassEx.lpszClassName = _T("WL_WEBVIEW");
    this->setup.wndClassEx.hbrBackground = wlx::sys_color_brush(COLOR_WINDOW);

    this->on_message(WM_CREATE, [this](wl::params) -> LRESULT {
        RECT rc{};
        GetClientRect(this->hwnd(), &rc);
        this->_urlBar.create(this->hwnd(), IDC_URLBAR, wl::textbox::type::NORMAL, {0, 0}, rc.right, URL_BAR_HEIGHT);
        SendMessage(this->_urlBar.hwnd(), EM_SETREADONLY, TRUE, 0);
        return 0;
    });

    this->on_message(WM_SIZE, [this](wl::params p) -> LRESULT {
        const int cx = LOWORD(p.lParam);
        if (this->_urlBar.hwnd()) {
            SetWindowPos(this->_urlBar.hwnd(), nullptr, 0, 0, cx, URL_BAR_HEIGHT, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
        }
        return 0;
    });

    this->on_message(WM_PAINT, [this](wl::params) -> LRESULT {
        PAINTSTRUCT ps{};
        HDC hdc = BeginPaint(this->hwnd(), &ps);

        RECT rc{};
        GetClientRect(this->hwnd(), &rc);

        // Thin separator line below the URL bar
        RECT lineRc{rc.left, URL_BAR_HEIGHT - 1, rc.right, URL_BAR_HEIGHT};
        FillRect(hdc, &lineRc, wlx::sys_color_brush(COLOR_BTNSHADOW));

        // Content placeholder area
        RECT contentRc{rc.left, URL_BAR_HEIGHT, rc.right, rc.bottom};
        FillRect(hdc, &contentRc, wlx::sys_color_brush(COLOR_WINDOW));

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
        DrawText(
            hdc,
            _T("[ Web Content Placeholder ]\r\n\r\n")
            _T("Replace wlx::webview with WebView2\r\nor IWebBrowser2 hosting."),
            -1,
            &contentRc,
            DT_CENTER | DT_VCENTER | DT_WORDBREAK
        );

        EndPaint(this->hwnd(), &ps);
        return 0;
    });
}

wlx::webview &wlx::webview::navigate(const wl::tstring &url) {
    this->_currentUrl = url;
    if (this->_urlBar.hwnd()) {
        SendMessage(this->_urlBar.hwnd(), WM_SETTEXT, 0, reinterpret_cast<LPARAM>(url.c_str()));
    }
    if (this->hwnd()) {
        InvalidateRect(this->hwnd(), nullptr, TRUE);
    }
    return *this;
}

const wl::tstring &wlx::webview::get_url() const noexcept {
    return this->_currentUrl;
}
