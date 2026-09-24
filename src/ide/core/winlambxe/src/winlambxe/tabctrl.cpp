#include "winlambxe/tabctrl.h"

RECT wlx::tabctrl::_close_btn_rect(const RECT &tabRc) noexcept {
    int cy = tabRc.bottom - tabRc.top;
    int btnTop = tabRc.top + (cy - CLOSE_BTN_SIZE) / 2;
    int btnRight = tabRc.right - CLOSE_BTN_MARGIN;
    return {btnRight - CLOSE_BTN_SIZE, btnTop, btnRight, btnTop + CLOSE_BTN_SIZE};
}

int wlx::tabctrl::_hit_test_close(POINT pt) const noexcept {
    if (!this->_closable)
        return -1;
    int n = TabCtrl_GetItemCount(this->_hWnd);
    for (int i = 0; i < n; ++i) {
        RECT tabRc{};
        TabCtrl_GetItemRect(this->_hWnd, i, &tabRc);
        const RECT closeRc = _close_btn_rect(tabRc);
        if (PtInRect(&closeRc, pt))
            return i;
    }
    return -1;
}

wl::tstring wlx::tabctrl::_pad_label(HWND hwnd, const wl::tstring &label, int extraPx) {
    if (!hwnd || extraPx <= 0)
        return label;
    HDC hdc = GetDC(hwnd);
    if (!hdc)
        return label;
    HFONT hFont = reinterpret_cast<HFONT>(SendMessage(hwnd, WM_GETFONT, 0, 0));
    HFONT hOld = hFont ? reinterpret_cast<HFONT>(SelectObject(hdc, hFont)) : nullptr;

    SIZE spaceSz{};
    GetTextExtentPoint32(hdc, _T(" "), 1, &spaceSz);
    int spaceW = (spaceSz.cx > 0) ? spaceSz.cx : 4;

    int count = (extraPx + spaceW - 1) / spaceW;
    if (count < 1)
        count = 1;

    if (hOld)
        SelectObject(hdc, hOld);
    ReleaseDC(hwnd, hdc);

    return label + wl::tstring(static_cast<size_t>(count), _T(' '));
}

void wlx::tabctrl::_refresh_padding() {
    if (!this->_hWnd)
        return;
    const int extraPx = this->_closable ? (CLOSE_BTN_SIZE + CLOSE_BTN_MARGIN * 2) : TAB_RIGHT_PAD;
    for (size_t i = 0; i < this->_labels.size(); ++i) {
        wl::tstring padded = _pad_label(this->_hWnd, this->_labels[i], extraPx);
        TCITEM tci{};
        tci.mask = TCIF_TEXT;
        tci.pszText = const_cast<TCHAR *>(padded.data());
        TabCtrl_SetItem(this->_hWnd, static_cast<int>(i), &tci);
    }
}

void wlx::tabctrl::_draw_close_x(HDC hdc, const RECT &r, COLORREF stroke) noexcept {
    int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
    if (dpiY <= 0)
        dpiY = 96;
    int penW = MulDiv(2, dpiY, 96);
    if (penW < 1)
        penW = 1;

    HPEN pen = CreatePen(PS_SOLID, penW, stroke);
    HPEN old = reinterpret_cast<HPEN>(SelectObject(hdc, pen));

    const int inset = 3;
    const int x1 = r.left + inset;
    const int y1 = r.top + inset;
    const int x2 = r.right - inset;
    const int y2 = r.bottom - inset;

    MoveToEx(hdc, x1, y1, nullptr);
    LineTo(hdc, x2, y2);
    MoveToEx(hdc, x2, y1, nullptr);
    LineTo(hdc, x1, y2);

    SelectObject(hdc, old);
    DeleteObject(pen);
}

wlx::tabctrl::tabctrl() : wl::wnd(_hWnd), base_native_ctrl_pubm(_baseNativeCtrl), base_focus_pubm(_hWnd) {
    // Register subclass handlers before install so _canAdd is still true.
    this->_subclass.on_message(WM_LBUTTONDOWN, [this](wl::params p) -> LRESULT {
        POINT pt{GET_X_LPARAM(p.lParam), GET_Y_LPARAM(p.lParam)};
        const int idx = this->_hit_test_close(pt);
        if (idx >= 0 && this->_onCloseTab) {
            this->_onCloseTab(static_cast<size_t>(idx));
            return 0;
        }
        // Not a close-button hit — delegate to the tab control's own WndProc.
        return DefSubclassProc(this->_hWnd, p.message, p.wParam, p.lParam);
    });

    this->_subclass.on_message(WM_MOUSEMOVE, [this](wl::params p) -> LRESULT {
        if (!this->_closable) {
            return DefSubclassProc(this->_hWnd, p.message, p.wParam, p.lParam);
        }
        POINT pt{GET_X_LPARAM(p.lParam), GET_Y_LPARAM(p.lParam)};
        const int idx = this->_hit_test_close(pt);

        if (idx != this->_hoveredCloseIdx) {
            if (this->_hoveredCloseIdx >= 0) {
                RECT r{};
                if (TabCtrl_GetItemRect(this->_hWnd, this->_hoveredCloseIdx, &r))
                    InvalidateRect(this->_hWnd, &r, FALSE);
            }
            this->_hoveredCloseIdx = idx;
            if (idx >= 0) {
                RECT r{};
                if (TabCtrl_GetItemRect(this->_hWnd, idx, &r))
                    InvalidateRect(this->_hWnd, &r, FALSE);
            }
        }

        if (!this->_mouseTracking) {
            TRACKMOUSEEVENT tme{};
            tme.cbSize = sizeof(tme);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = this->_hWnd;
            TrackMouseEvent(&tme);
            this->_mouseTracking = true;
        }
        return DefSubclassProc(this->_hWnd, p.message, p.wParam, p.lParam);
    });

    this->_subclass.on_message(WM_MOUSELEAVE, [this](wl::params p) -> LRESULT {
        this->_mouseTracking = false;
        if (this->_hoveredCloseIdx >= 0) {
            RECT r{};
            if (TabCtrl_GetItemRect(this->_hWnd, this->_hoveredCloseIdx, &r))
                InvalidateRect(this->_hWnd, &r, FALSE);
            this->_hoveredCloseIdx = -1;
        }
        return DefSubclassProc(this->_hWnd, p.message, p.wParam, p.lParam);
    });
}

wlx::tabctrl &wlx::tabctrl::create(const wl::wnd *parent, int ctrlId, POINT pos, SIZE size) {
    return this->create(parent->hwnd(), ctrlId, pos, size);
}

wlx::tabctrl &wlx::tabctrl::create(HWND hParent, int ctrlId, POINT pos, SIZE size) {
    this->_baseNativeCtrl.create(hParent, ctrlId, nullptr, pos, size, WC_TABCONTROL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_OWNERDRAWFIXED, 0);
    // Subclass to intercept WM_LBUTTONDOWN / WM_MOUSEMOVE / WM_MOUSELEAVE.
    this->_subclass.install_subclass(this->_hWnd);
    return *this;
}

wlx::tabctrl &wlx::tabctrl::add_tab(const wl::tstring &label) {
    return this->add_tab(label.c_str());
}

wlx::tabctrl &wlx::tabctrl::add_tab(const TCHAR *label) {
    this->_labels.emplace_back(label);
    const int extraPx = this->_closable ? (CLOSE_BTN_SIZE + CLOSE_BTN_MARGIN * 2) : TAB_RIGHT_PAD;
    wl::tstring padded = _pad_label(this->_hWnd, this->_labels.back(), extraPx);
    TCITEM tci{};
    tci.mask = TCIF_TEXT;
    tci.pszText = const_cast<TCHAR *>(padded.empty() ? label : padded.data());
    TabCtrl_InsertItem(this->_hWnd, static_cast<int>(this->_labels.size() - 1), &tci);
    return *this;
}

wlx::tabctrl &wlx::tabctrl::remove_tab(size_t index) {
    if (index < this->_labels.size()) {
        TabCtrl_DeleteItem(this->_hWnd, static_cast<int>(index));
        this->_labels.erase(this->_labels.begin() + static_cast<ptrdiff_t>(index));
        // A removed tab cannot stay "hovered"; reset to avoid drawing hover
        // on a stale (or now shifted) index.
        this->_hoveredCloseIdx = -1;
    }
    return *this;
}

int wlx::tabctrl::selected_index() const noexcept {
    return TabCtrl_GetCurSel(this->_hWnd);
}

wlx::tabctrl &wlx::tabctrl::select(size_t index) noexcept {
    TabCtrl_SetCurSel(this->_hWnd, static_cast<int>(index));
    return *this;
}

size_t wlx::tabctrl::count() const noexcept {
    return static_cast<size_t>(TabCtrl_GetItemCount(this->_hWnd));
}

RECT wlx::tabctrl::display_area() const noexcept {
    RECT rc{};
    GetClientRect(this->_hWnd, &rc);
    TabCtrl_AdjustRect(this->_hWnd, FALSE, &rc);
    return rc;
}

wlx::tabctrl &wlx::tabctrl::on_close_tab(std::function<void(size_t)> fn) {
    this->_onCloseTab = std::move(fn);
    return *this;
}

wlx::tabctrl &wlx::tabctrl::closable(bool yes) noexcept {
    if (this->_closable == yes)
        return *this;
    this->_closable = yes;
    if (!yes) {
        this->_hoveredCloseIdx = -1;
    }
    if (this->_hWnd) {
        this->_refresh_padding();
        InvalidateRect(this->_hWnd, nullptr, TRUE);
    }
    return *this;
}

bool wlx::tabctrl::closable() const noexcept {
    return this->_closable;
}

void wlx::tabctrl::draw_item(const DRAWITEMSTRUCT &dis) noexcept {
    HDC hdc = dis.hDC;
    RECT rc = dis.rcItem;
    bool sel = (dis.itemState & ODS_SELECTED) != 0;

    // Background
    HBRUSH hBg = CreateSolidBrush(GetSysColor(sel ? COLOR_BTNFACE : COLOR_BTNHIGHLIGHT));
    FillRect(hdc, &rc, hBg);
    DeleteObject(hBg);

    // Use the font assigned via WM_SETFONT so labels render at the
    // system DPI scale (the default HDC font is the bitmap SYSTEM_FONT).
    HFONT hFont = reinterpret_cast<HFONT>(SendMessage(this->_hWnd, WM_GETFONT, 0, 0));
    HFONT hOldFont = hFont ? reinterpret_cast<HFONT>(SelectObject(hdc, hFont)) : nullptr;

    // Label text — reserve right margin for the close button when present.
    RECT textRc = rc;
    textRc.left += 6;
    textRc.right = this->_closable ? (rc.right - CLOSE_BTN_SIZE - CLOSE_BTN_MARGIN * 2) : (rc.right - 6);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));

    static const wl::tstring kEmptyLabel;
    const wl::tstring &label = (dis.itemID < this->_labels.size()) ? this->_labels[dis.itemID] : kEmptyLabel;
    DrawText(hdc, label.c_str(), -1, &textRc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    // Close button — drawn as two GDI diagonals so it always reads as an X.
    if (this->_closable) {
        RECT closeRc = _close_btn_rect(rc);
        const bool hovered = (static_cast<int>(dis.itemID) == this->_hoveredCloseIdx);
        COLORREF stroke = hovered ? RGB(20, 20, 20) : RGB(90, 90, 90);
        if (hovered) {
            HBRUSH hb = CreateSolidBrush(RGB(220, 220, 220));
            FillRect(hdc, &closeRc, hb);
            DeleteObject(hb);
        }
        _draw_close_x(hdc, closeRc, stroke);
    }

    if (hOldFont)
        SelectObject(hdc, hOldFont);

    // Focus rectangle
    if (dis.itemState & ODS_FOCUS) {
        DrawFocusRect(hdc, &rc);
    }
}
