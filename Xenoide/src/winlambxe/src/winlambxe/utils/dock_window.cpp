#include "winlambxe/utils/dock_window.h"
#include "winlambxe/utils/sys_brush.h"

wlx::dock_window::dock_window()
{
	this->setup.wndClassEx.lpszClassName = _T("XENOIDEW_DOCK_WINDOW");
	this->setup.wndClassEx.hbrBackground = wlx::sys_color_brush(COLOR_BTNFACE);

	this->on_message(WM_CREATE, [this](wl::params) -> LRESULT {
		// Build a system UI font (Tahoma/Segoe UI), pre-scaled to the
		// process system DPI. Used for the painted title and applied
		// to the tab control so its labels render correctly.
		this->_font.create_ui();
		this->_recompute_metrics();
		RECT rc{};
		GetClientRect(this->hwnd(), &rc);
		const int w = rc.right;
		this->_tabs.create(this->hwnd(), IDC_TABS,
			{ 0, this->_titleH }, { w, this->_tabH });
		this->_tabs.closable(false);
		SendMessage(this->_tabs.hwnd(), WM_SETFONT,
			reinterpret_cast<WPARAM>(this->_font.hfont()), TRUE);
		return 0;
		});

	this->on_message(WM_SIZE, [this](wl::params p) -> LRESULT {
		if (p.wParam == SIZE_MINIMIZED) return 0;
		this->_layout(LOWORD(p.lParam), HIWORD(p.lParam));
		return 0;
		});

	this->on_message(WM_PAINT, [this](wl::params) -> LRESULT {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(this->hwnd(), &ps);
		RECT rc{};
		GetClientRect(this->hwnd(), &rc);
		RECT title{ 0, 0, rc.right, _titleH };
		this->_paint_title(hdc, title);
		EndPaint(this->hwnd(), &ps);
		return 0;
		});

	// Required by wlx::tabctrl when TCS_OWNERDRAWFIXED is set: forward
	// WM_DRAWITEM to the tab control's draw_item().
	this->on_message(WM_DRAWITEM, [this](wl::params p) -> LRESULT {
		const auto& dis = *reinterpret_cast<DRAWITEMSTRUCT*>(p.lParam);
		if (dis.CtlID == IDC_TABS) {
			this->_tabs.draw_item(dis);
			return TRUE;
		}
		return 0;
		});

	this->on_message(WM_NOTIFY, [this](wl::params p) -> LRESULT {
		const auto* nmh = reinterpret_cast<NMHDR*>(p.lParam);
		if (nmh && nmh->idFrom == IDC_TABS && nmh->code == TCN_SELCHANGE) {
			int idx = this->_tabs.selected_index();
			if (idx >= 0) this->select_page(static_cast<size_t>(idx));
		}
		return 0;
		});
}

wlx::dock_window& wlx::dock_window::set_title(wl::tstring text)
{
	_title = std::move(text);
	if (this->hwnd()) {
		RECT rc{ 0, 0, 0, 0 };
		GetClientRect(this->hwnd(), &rc);
		rc.bottom = _titleH;
		InvalidateRect(this->hwnd(), &rc, TRUE);
	}
	return *this;
}

wlx::dock_window& wlx::dock_window::add_page(const wl::tstring& label, HWND page)
{
	if (!page) return *this;
	SetParent(page, this->hwnd());
	ShowWindow(page, SW_HIDE);
	_pages.push_back(page);
	_tabs.add_tab(label.c_str());
	if (_activeIdx < 0) {
		this->select_page(0);
	}
	else {
		RECT rc{};
		GetClientRect(this->hwnd(), &rc);
		this->_layout(rc.right, rc.bottom);
	}
	return *this;
}

wlx::dock_window& wlx::dock_window::select_page(size_t index)
{
	if (index >= _pages.size()) return *this;
	if (_activeIdx >= 0 && static_cast<size_t>(_activeIdx) < _pages.size()) {
		ShowWindow(_pages[static_cast<size_t>(_activeIdx)], SW_HIDE);
	}
	_activeIdx = static_cast<int>(index);
	_tabs.select(index);
	ShowWindow(_pages[index], SW_SHOW);
	RECT rc{};
	GetClientRect(this->hwnd(), &rc);
	this->_layout(rc.right, rc.bottom);
	RECT title{ 0, 0, rc.right, _titleH };
	InvalidateRect(this->hwnd(), &title, TRUE);
	return *this;
}

int wlx::dock_window::selected_index() const noexcept
{
	return _activeIdx;
}

void wlx::dock_window::_recompute_metrics() noexcept
{
	if (!this->_font.hfont()) return;
	HDC hdc = GetDC(this->hwnd());
	if (!hdc) return;
	HFONT old = reinterpret_cast<HFONT>(SelectObject(hdc, this->_font.hfont()));
	TEXTMETRIC tm{};
	if (GetTextMetrics(hdc, &tm)) {
		const int h = tm.tmHeight + 8;
		this->_titleH = h;
		this->_tabH = h;
	}
	SelectObject(hdc, old);
	ReleaseDC(this->hwnd(), hdc);
}

void wlx::dock_window::_layout(int w, int h) noexcept
{
	if (w <= 0 || h <= 0) return;
	const int contentTop = _titleH + _tabH;
	const int contentH = (h > contentTop) ? h - contentTop : 0;

	defer_window_pos dwp(2);
	if (_tabs.hwnd()) {
		dwp.defer(_tabs.hwnd(), nullptr,
			0, _titleH, w, _tabH,
			SWP_NOZORDER | SWP_NOACTIVATE);
	}
	if (_activeIdx >= 0 && static_cast<size_t>(_activeIdx) < _pages.size()) {
		dwp.defer(_pages[static_cast<size_t>(_activeIdx)], nullptr,
			0, contentTop, w, contentH,
			SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

void wlx::dock_window::_paint_title(HDC hdc, const RECT& rc) noexcept
{
	HBRUSH bg = CreateSolidBrush(GetSysColor(COLOR_ACTIVECAPTION));
	FillRect(hdc, &rc, bg);
	DeleteObject(bg);

	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, GetSysColor(COLOR_CAPTIONTEXT));
	HFONT old = nullptr;
	if (this->_font.hfont()) {
		old = reinterpret_cast<HFONT>(SelectObject(hdc, this->_font.hfont()));
	}
	RECT tx = rc;
	tx.left += 6;
	DrawText(hdc, _title.c_str(), -1, &tx,
		DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	if (old) SelectObject(hdc, old);
}
