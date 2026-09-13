#include "winlambxe/utils/splitter_vertical.h"
#include "winlambxe/utils/sys_brush.h"

namespace wlx {

splitter_vertical::splitter_vertical()
{
	this->setup.wndClassEx.lpszClassName = _T("WINLAMB_SPLITTER_VERT");
	this->setup.style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS;

	this->on_message(WM_PAINT, [this](wl::params) -> LRESULT {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(this->hwnd(), &ps);
		RECT rc;
		GetClientRect(this->hwnd(), &rc);
		// Paint only the background. The child edit windows will cover the rest.
		FillRect(hdc, &rc, sys_color_brush(COLOR_BTNFACE));
		EndPaint(this->hwnd(), &ps);
		return 0;
		});

	this->setup_handlers();
}

LPTSTR splitter_vertical::get_cursor() const noexcept
{
	return IDC_SIZEWE;
}

int splitter_vertical::get_primary_size(int w, int h) const noexcept
{
	(void)h;
	return w;
}

void splitter_vertical::on_layout(int w, int h) noexcept
{
	if (_split_pos == -1) {
		_split_pos = (w - _split_thickness) / 2;
	}

	if (_split_pos < 0) _split_pos = 0;
	if (_split_pos > w - _split_thickness) _split_pos = w - _split_thickness;
	if (_split_pos < 0) _split_pos = 0; // safe guard if w is very small

	defer_window_pos dwp(2);
	dwp.defer(_hwnd1, nullptr, 0, 0, _split_pos, h, SWP_NOZORDER | SWP_NOACTIVATE);
	dwp.defer(_hwnd2, nullptr, _split_pos + _split_thickness, 0, w - _split_pos - _split_thickness, h, SWP_NOZORDER | SWP_NOACTIVATE);
}

void splitter_vertical::on_drag(int x, int y) noexcept
{
	(void)y; // Vertical bar moves horizontally only

	RECT rc;
	GetClientRect(this->hwnd(), &rc);
	int w = rc.right;

	// Ensure bounds
	int min_x = 10;
	int max_x = w - _split_thickness - 10;

	int new_x = x;
	if (new_x < min_x) new_x = min_x;
	if (new_x > max_x) new_x = max_x;

	_split_pos = new_x;
	on_layout(rc.right, rc.bottom);
}

} // namespace wlx
