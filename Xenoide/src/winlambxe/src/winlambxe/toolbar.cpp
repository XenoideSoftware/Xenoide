#include "toolbar.h"

#include <cassert>

void wlx::toolbar::_ensure_loaded() noexcept
{
	static bool initialised = []() -> bool {
		INITCOMMONCONTROLSEX icc{};
		icc.dwSize = sizeof(icc);
		icc.dwICC  = ICC_BAR_CLASSES; // WC_TOOLBAR + tooltips
		InitCommonControlsEx(&icc);
		return true;
		}();
	(void)initialised;
}

wlx::toolbar::toolbar() :
	wl::wnd(_hWnd),
	base_native_ctrl_pubm(_baseNativeCtrl),
	base_focus_pubm(_hWnd)
{

}

wlx::toolbar& wlx::toolbar::create(const wl::wnd* parent, int ctrlId, POINT pos, SIZE size)
{
	return this->create(parent->hwnd(), ctrlId, pos, size);
}

wlx::toolbar& wlx::toolbar::create(HWND hParent, int ctrlId, POINT pos, SIZE size)
{
	_ensure_loaded();
	this->_baseNativeCtrl.create(hParent, ctrlId, nullptr, pos, size,
		TOOLBARCLASSNAME,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
		TBSTYLE_TOOLTIPS | TBSTYLE_WRAPABLE | CCS_NODIVIDER | CCS_NOPARENTALIGN | CCS_NORESIZE,
		0);

	// Give the toolbar its standard button width/height before any buttons are
	// added so the first auto_size() picks consistent metrics.
	SendMessageW(this->_hWnd, TB_SETPADDING, 0, MAKELPARAM(6, 6));

	// Install the system-standard image list so STD_FILENEW / STD_FILEOPEN / etc.
	// resolve out of the box. User-supplied lists override via set_image_list().
	HIMAGELIST hil = reinterpret_cast<HIMAGELIST>(
		SendMessageW(this->_hWnd, TB_LOADIMAGES,
			IDB_STD_SMALL_COLOR,
			reinterpret_cast<LPARAM>(HINST_COMMCTRL)));
	if (hil) {
		SendMessageW(this->_hWnd, TB_SETIMAGELIST, 0,
			reinterpret_cast<LPARAM>(hil));
	}
	return *this;
}

// ====================================================================
// Buttons
// ====================================================================

wlx::toolbar& wlx::toolbar::add_button(int idCommand, int stdBitmap, const TCHAR* label,
	BYTE styles, BYTE state)
{
	SendMessage(this->_hWnd, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);

	TBBUTTON tbb{};

	tbb.iBitmap = stdBitmap;
	tbb.idCommand = idCommand;
	tbb.fsState = state;
	tbb.fsStyle = styles;
	tbb.iString = reinterpret_cast<INT_PTR>(label ? label : _T(""));
	LRESULT r = SendMessageW(this->_hWnd, TB_ADDBUTTONS, 1,
		reinterpret_cast<LPARAM>(&tbb));

	assert(r == TRUE);

	return *this;
}

wlx::toolbar& wlx::toolbar::add_button(int idCommand, int stdBitmap, const wl::tstring& label,
	BYTE styles, BYTE state)
{
	return this->add_button(idCommand, stdBitmap, label.c_str(), styles, state);
}

wlx::toolbar& wlx::toolbar::add_button_bitmap(int idCommand, int bitmapIndex, const TCHAR* label,
	BYTE styles, BYTE state)
{
	TBBUTTON tbb{};
	tbb.iBitmap = bitmapIndex;
	tbb.idCommand = idCommand;
	tbb.fsState = state;
	tbb.fsStyle = styles;
	tbb.iString = reinterpret_cast<INT_PTR>(label ? label : _T(""));
	LRESULT r = SendMessageW(this->_hWnd, TB_ADDBUTTONS, 1,
		reinterpret_cast<LPARAM>(&tbb));

	assert(r == TRUE);

	return *this;
}

wlx::toolbar& wlx::toolbar::add_separator(int width)
{
	TBBUTTON tbb{};
	tbb.iBitmap = 0;
	tbb.idCommand = 0;
	tbb.fsState = TBSTATE_ENABLED;
	tbb.fsStyle = BTNS_SEP;
	if (width > 0) {
		SendMessageW(this->_hWnd, TB_SETBUTTONWIDTH, 0, MAKELPARAM(width, width));
	}
	SendMessageW(this->_hWnd, TB_ADDBUTTONS, 1,
		reinterpret_cast<LPARAM>(&tbb));
	// Restore default auto button width so subsequent buttons aren't affected.
	SendMessageW(this->_hWnd, TB_SETBUTTONWIDTH, 0, 0);
	return *this;
}

wlx::toolbar& wlx::toolbar::insert_button(int index, int idCommand, int stdBitmap,
	const TCHAR* label, BYTE styles, BYTE state)
{
	if (index < 0) return this->add_button(idCommand, stdBitmap, label, styles, state);
	TBBUTTON tbb{};
	tbb.iBitmap = stdBitmap;
	tbb.idCommand = idCommand;
	tbb.fsState = state;
	tbb.fsStyle = styles;
	tbb.iString = reinterpret_cast<INT_PTR>(label ? label : _T(""));
	LRESULT r = SendMessageW(this->_hWnd, TB_INSERTBUTTON, static_cast<WPARAM>(index),
		reinterpret_cast<LPARAM>(&tbb));

	assert(r == TRUE);

	return *this;
}

wlx::toolbar& wlx::toolbar::delete_button(int idCommand) noexcept
{
	const int idx = this->button_index(idCommand);
	if (idx >= 0) {
		SendMessageW(this->_hWnd, TB_DELETEBUTTON, static_cast<WPARAM>(idx), 0);
	}
	return *this;
}

int wlx::toolbar::button_index(int idCommand) const noexcept
{
	return static_cast<int>(
		SendMessageW(this->_hWnd, TB_COMMANDTOINDEX,
			static_cast<WPARAM>(idCommand), 0));
}

int wlx::toolbar::button_count() const noexcept
{
	return static_cast<int>(
		SendMessageW(this->_hWnd, TB_BUTTONCOUNT, 0, 0));
}

TBBUTTON wlx::toolbar::button_info(int index) const
{
	TBBUTTON tbb{};
	if (SendMessageW(this->_hWnd, TB_GETBUTTON,
		static_cast<WPARAM>(index),
		reinterpret_cast<LPARAM>(&tbb)) == FALSE) {
		throw std::runtime_error("TB_GETBUTTON failed");
	}
	return tbb;
}

wlx::toolbar& wlx::toolbar::set_button_info(int index, int cmd, int bitmap) noexcept
{
	TBBUTTONINFO tbbi{};
	tbbi.cbSize = sizeof(tbbi);
	tbbi.dwMask = 0;
	if (cmd >= 0)    { tbbi.dwMask |= TBIF_COMMAND; tbbi.idCommand = cmd; }
	if (bitmap >= 0) { tbbi.dwMask |= TBIF_IMAGE;   tbbi.iImage    = bitmap; }
	if (tbbi.dwMask != 0) {
		SendMessageW(this->_hWnd, TB_SETBUTTONINFO,
			static_cast<WPARAM>(index),
			reinterpret_cast<LPARAM>(&tbbi));
	}
	return *this;
}

// ====================================================================
// Button state
// ====================================================================

wlx::toolbar& wlx::toolbar::enable_button(int idCommand, bool enable) noexcept
{
	SendMessageW(this->_hWnd, TB_ENABLEBUTTON,
		static_cast<WPARAM>(idCommand),
		enable ? MAKELPARAM(TBSTATE_ENABLED, 0)
		       : MAKELPARAM(0, 0));
	return *this;
}

wlx::toolbar& wlx::toolbar::check_button(int idCommand, bool check) noexcept
{
	SendMessageW(this->_hWnd, TB_CHECKBUTTON,
		static_cast<WPARAM>(idCommand),
		check ? MAKELPARAM(TBSTATE_CHECKED, 0)
		      : MAKELPARAM(0, 0));
	return *this;
}

wlx::toolbar& wlx::toolbar::press_button(int idCommand, bool press) noexcept
{
	SendMessageW(this->_hWnd, TB_PRESSBUTTON,
		static_cast<WPARAM>(idCommand),
		press ? MAKELPARAM(TBSTATE_PRESSED, 0)
		      : MAKELPARAM(0, 0));
	return *this;
}

wlx::toolbar& wlx::toolbar::hide_button(int idCommand, bool hide) noexcept
{
	SendMessageW(this->_hWnd, TB_HIDEBUTTON,
		static_cast<WPARAM>(idCommand),
		hide ? MAKELPARAM(TBSTATE_HIDDEN, 0)
		     : MAKELPARAM(0, 0));
	return *this;
}

BYTE wlx::toolbar::button_state(int idCommand) const noexcept
{
	TBBUTTONINFO tbbi{};
	tbbi.cbSize = sizeof(tbbi);
	tbbi.dwMask = TBIF_STATE;
	if (SendMessageW(this->_hWnd, TB_GETBUTTONINFO,
		static_cast<WPARAM>(idCommand),
		reinterpret_cast<LPARAM>(&tbbi)) == -1) {
		return 0;
	}
	return tbbi.fsState;
}

// ====================================================================
// Image lists
// ====================================================================

wlx::toolbar& wlx::toolbar::set_image_list(HIMAGELIST himl) noexcept
{
	SendMessageW(this->_hWnd, TB_SETIMAGELIST, 0,
		reinterpret_cast<LPARAM>(himl));
	return *this;
}

wlx::toolbar& wlx::toolbar::set_disabled_image_list(HIMAGELIST himl) noexcept
{
	SendMessageW(this->_hWnd, TB_SETDISABLEDIMAGELIST, 0,
		reinterpret_cast<LPARAM>(himl));
	return *this;
}

wlx::toolbar& wlx::toolbar::set_hot_image_list(HIMAGELIST himl) noexcept
{
	SendMessageW(this->_hWnd, TB_SETHOTIMAGELIST, 0,
		reinterpret_cast<LPARAM>(himl));
	return *this;
}

wlx::toolbar& wlx::toolbar::add_std_bitmaps(int resId, int n) noexcept
{
	SendMessageW(this->_hWnd, TB_LOADIMAGES,
		static_cast<WPARAM>(resId),
		reinterpret_cast<LPARAM>(HINST_COMMCTRL));
	(void)n; // count is informational; TB_LOADIMAGES loads the full set for resId.
	return *this;
}

wlx::toolbar& wlx::toolbar::add_user_bitmaps(HBITMAP hBmp, int n) noexcept
{
	SendMessageW(this->_hWnd, TB_ADDBITMAP,
		static_cast<WPARAM>(n),
		reinterpret_cast<LPARAM>(&hBmp));
	return *this;
}

// ====================================================================
// Sizing & layout
// ====================================================================

wlx::toolbar& wlx::toolbar::set_button_size(int width, int height) noexcept
{
	SendMessageW(this->_hWnd, TB_SETBUTTONSIZE, 0,
		MAKELPARAM(width, height));
	return *this;
}

wlx::toolbar& wlx::toolbar::set_padding(int cx, int cy) noexcept
{
	SendMessageW(this->_hWnd, TB_SETPADDING, 0,
		MAKELPARAM(cx, cy));
	return *this;
}

wlx::toolbar& wlx::toolbar::auto_size() noexcept
{
	SendMessageW(this->_hWnd, TB_AUTOSIZE, 0, 0);
	DWORD dw = 0; SendMessageW(this->_hWnd, TB_GETBUTTONSIZE, 0, 0);
	(void)dw;
	return *this;
}

int wlx::toolbar::height() const noexcept
{
	DWORD dw = static_cast<DWORD>(
		SendMessageW(this->_hWnd, TB_GETBUTTONSIZE, 0, 0));
	return static_cast<int>(HIWORD(dw));
}

int wlx::toolbar::width() const noexcept
{
	DWORD dw = static_cast<DWORD>(
		SendMessageW(this->_hWnd, TB_GETBUTTONSIZE, 0, 0));
	int btnW = static_cast<int>(LOWORD(dw));
	int btnH = static_cast<int>(HIWORD(dw));
	int perRow = btnH > 0 ? 1 : 1;
	(void)perRow;
	int n = this->button_count();
	return btnW * n;
}

wlx::toolbar& wlx::toolbar::set_extended_style(DWORD mask, DWORD extendedStyle) noexcept
{
	SendMessageW(this->_hWnd, TB_SETEXTENDEDSTYLE,
		static_cast<WPARAM>(mask),
		static_cast<LPARAM>(extendedStyle));
	return *this;
}

DWORD wlx::toolbar::extended_style() const noexcept
{
	return static_cast<DWORD>(
		SendMessageW(this->_hWnd, TB_GETEXTENDEDSTYLE, 0, 0));
}

// ====================================================================
// Tooltips
// ====================================================================

HWND wlx::toolbar::tooltip_hwnd() const noexcept
{
	return reinterpret_cast<HWND>(
		SendMessageW(this->_hWnd, TB_GETTOOLTIPS, 0, 0));
}

wlx::toolbar& wlx::toolbar::set_tooltip(int idCommand, const TCHAR* text) noexcept
{
	TOOLINFO ti{};
	ti.cbSize = sizeof(ti);
	ti.hwnd   = this->_hWnd;
	ti.uId    = static_cast<UINT_PTR>(idCommand);
	ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;

	// Tooltips use a counted string; copy to a local buffer of TI's limit.
	ti.lpszText = const_cast<TCHAR*>(text);

	HWND hTip = this->tooltip_hwnd();
	if (hTip) {
		SendMessageW(hTip, TTM_UPDATETIPTEXT, 0, reinterpret_cast<LPARAM>(&ti));
	}
	return *this;
}

wlx::toolbar& wlx::toolbar::set_tooltip_max_width(int pixels) noexcept
{
	HWND hTip = this->tooltip_hwnd();
	if (hTip) {
		SendMessageW(hTip, TTM_SETMAXTIPWIDTH, 0,
			static_cast<LPARAM>(pixels));
	}
	return *this;
}
