#include "winlambxe/utils/document_tabs.h"
#include "winlambxe/utils/sys_brush.h"

namespace wlx {

    document_tabs::document_tabs() {
        this->setup.wndClassEx.lpszClassName = _T("XENOIDEW_DOCUMENT_TABS");
        this->setup.wndClassEx.hbrBackground = sys_color_brush(COLOR_BTNFACE);

        this->on_message(WM_CREATE, [this](wl::params) -> LRESULT {
            // Build a system UI font and apply it to the tab control so
            // labels render at the system DPI scale.
            this->_font.create_ui();
            this->_recompute_metrics();
            RECT rc{};
            GetClientRect(this->hwnd(), &rc);
            this->_tabs.create(this->hwnd(), IDC_TABS, {0, 0}, {rc.right, this->_tabH});
            SendMessage(this->_tabs.hwnd(), WM_SETFONT, reinterpret_cast<WPARAM>(this->_font.hfont()), TRUE);
            this->_tabs.on_close_tab([this](size_t i) { this->_close_document(i); });
            return 0;
        });

        this->on_message(WM_SIZE, [this](wl::params p) -> LRESULT {
            if (p.wParam == SIZE_MINIMIZED)
                return 0;
            this->_layout(LOWORD(p.lParam), HIWORD(p.lParam));
            return 0;
        });

        this->on_message(WM_DRAWITEM, [this](wl::params p) -> LRESULT {
            const auto &dis = *reinterpret_cast<DRAWITEMSTRUCT *>(p.lParam);
            if (dis.CtlID == IDC_TABS) {
                this->_tabs.draw_item(dis);
                return TRUE;
            }
            return 0;
        });

        this->on_message(WM_NOTIFY, [this](wl::params p) -> LRESULT {
            const auto *nmh = reinterpret_cast<NMHDR *>(p.lParam);
            if (nmh && nmh->idFrom == IDC_TABS && nmh->code == TCN_SELCHANGE) {
                int idx = this->_tabs.selected_index();
                if (idx >= 0)
                    this->select_document(static_cast<size_t>(idx));
            }
            return 0;
        });
    }

    document_tabs &document_tabs::add_document(const wl::tstring &label, HWND page) {
        if (!page)
            return *this;
        SetParent(page, this->hwnd());
        ShowWindow(page, SW_HIDE);
        _pages.push_back(page);
        _tabs.add_tab(label.c_str());
        if (_activeIdx < 0) {
            this->select_document(0);
        } else {
            RECT rc{};
            GetClientRect(this->hwnd(), &rc);
            this->_layout(rc.right, rc.bottom);
        }
        return *this;
    }

    document_tabs &document_tabs::select_document(size_t index) {
        if (index >= _pages.size())
            return *this;
        if (_activeIdx >= 0 && static_cast<size_t>(_activeIdx) < _pages.size()) {
            ShowWindow(_pages[static_cast<size_t>(_activeIdx)], SW_HIDE);
        }
        _activeIdx = static_cast<int>(index);
        _tabs.select(index);
        ShowWindow(_pages[index], SW_SHOW);
        RECT rc{};
        GetClientRect(this->hwnd(), &rc);
        this->_layout(rc.right, rc.bottom);
        return *this;
    }

    int document_tabs::selected_index() const noexcept {
        return _activeIdx;
    }

    size_t document_tabs::count() const noexcept {
        return _pages.size();
    }

    document_tabs &document_tabs::on_close_document(std::function<void(size_t, HWND)> cb) {
        _onClose = std::move(cb);
        return *this;
    }

    void document_tabs::_recompute_metrics() noexcept {
        if (!this->_font.hfont())
            return;
        HDC hdc = GetDC(this->hwnd());
        if (!hdc)
            return;
        HFONT old = reinterpret_cast<HFONT>(SelectObject(hdc, this->_font.hfont()));
        TEXTMETRIC tm{};
        if (GetTextMetrics(hdc, &tm)) {
            this->_tabH = tm.tmHeight + 8;
        }
        SelectObject(hdc, old);
        ReleaseDC(this->hwnd(), hdc);
    }

    void document_tabs::_close_document(size_t index) {
        if (index >= _pages.size())
            return;
        HWND closed = _pages[index];

        if (_onClose)
            _onClose(index, closed);

        ShowWindow(closed, SW_HIDE);
        _tabs.remove_tab(index);
        _pages.erase(_pages.begin() + static_cast<ptrdiff_t>(index));

        if (_pages.empty()) {
            _activeIdx = -1;
            InvalidateRect(this->hwnd(), nullptr, TRUE);
        } else {
            size_t next = (index < _pages.size()) ? index : _pages.size() - 1;
            _activeIdx = -1;
            this->select_document(next);
        }
    }

    void document_tabs::_layout(int w, int h) noexcept {
        if (w <= 0 || h <= 0)
            return;
        const int contentTop = _tabH;
        const int contentH = (h > contentTop) ? h - contentTop : 0;

        defer_window_pos dwp(2);
        if (_tabs.hwnd()) {
            dwp.defer(_tabs.hwnd(), nullptr, 0, 0, w, _tabH, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        if (_activeIdx >= 0 && static_cast<size_t>(_activeIdx) < _pages.size()) {
            dwp.defer(_pages[static_cast<size_t>(_activeIdx)], nullptr, 0, contentTop, w, contentH, SWP_NOZORDER | SWP_NOACTIVATE);
        }
    }

} // namespace wlx
