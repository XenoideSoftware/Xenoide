#include "winlambxe/richedit.h"

// MSFTEDIT_CLASS (msftedit.dll, "RICHEDIT50W") is XP SP1+ and wide-only. On
// older toolchains it may not be declared, and on ANSI builds it is the wrong
// class name (msftedit.dll registers no ANSI class). _ensure_loaded() falls
// back to riched20.dll, which registers both RichEdit20A and RichEdit20W.
#if defined(UNICODE) || defined(_UNICODE)
#if defined(MSFTEDIT_CLASS)
#define WLX_RICHEDIT_CLASS MSFTEDIT_CLASS
#else
#define WLX_RICHEDIT_CLASS _T("RichEdit20W")
#endif
#else
#define WLX_RICHEDIT_CLASS _T("RichEdit20A")
#endif

HMODULE wlx::richedit::_ensure_loaded() {
    static HMODULE h = []() -> HMODULE {
        HMODULE mod = LoadLibrary(_T("Msftedit.dll"));
        if (!mod)
            mod = LoadLibrary(_T("riched20.dll"));
        return mod;
    }();
    if (!h)
        throw std::runtime_error("Failed to load RichEdit DLL");
    return h;
}

wlx::richedit::richedit() : wl::wnd(_hWnd), base_native_ctrl_pubm(_baseNativeCtrl), base_focus_pubm(_hWnd) {
}

wlx::richedit &wlx::richedit::create(const wl::wnd *parent, int ctrlId, POINT pos, SIZE size) {
    return this->create(parent->hwnd(), ctrlId, pos, size);
}

wlx::richedit &wlx::richedit::create(HWND hParent, int ctrlId, POINT pos, SIZE size) {
    _ensure_loaded();
    this->_baseNativeCtrl
        .create(hParent, ctrlId, nullptr, pos, size, WLX_RICHEDIT_CLASS, WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL | ES_WANTRETURN, WS_EX_CLIENTEDGE);
    return *this;
}

wlx::richedit &wlx::richedit::set_text(const tstring &text) noexcept {
    SetWindowText(this->_hWnd, text.c_str());
    return *this;
}

wlx::tstring wlx::richedit::get_text() const {
    const int len = GetWindowTextLength(this->_hWnd);
    if (!len)
        return {};
    tstring buf(static_cast<size_t>(len) + 1, _T('\0'));
    GetWindowText(this->_hWnd, const_cast<TCHAR *>(buf.data()), len + 1);
    buf.resize(static_cast<size_t>(len));
    return buf;
}

wlx::richedit &wlx::richedit::append_text(const tstring &text) noexcept {
    SendMessage(this->_hWnd, EM_SETSEL, static_cast<WPARAM>(-1), static_cast<LPARAM>(-1));
    SendMessage(this->_hWnd, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(text.c_str()));
    return *this;
}

wlx::richedit &wlx::richedit::set_font(LPCTSTR face, int sizePt) noexcept {
    CHARFORMAT2 cf{};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_FACE | CFM_SIZE;
    cf.yHeight = sizePt * 20; // half-points
    _tcsncpy_s(cf.szFaceName, face, LF_FACESIZE - 1);
    SendMessage(this->_hWnd, EM_SETCHARFORMAT, SCF_ALL, reinterpret_cast<LPARAM>(&cf));
    return *this;
}

wlx::richedit &wlx::richedit::set_readonly(bool ro) noexcept {
    SendMessage(this->_hWnd, EM_SETREADONLY, ro ? TRUE : FALSE, 0);
    return *this;
}
