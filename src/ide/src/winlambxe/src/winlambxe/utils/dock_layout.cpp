#include "winlambxe/utils/dock_layout.h"
#include "winlambxe/utils/sys_brush.h"

#include <algorithm>

namespace wlx {

dock_layout::dock_layout()
{
	this->setup.wndClassEx.lpszClassName = _T("XENOIDEW_DOCK_LAYOUT");
	this->setup.wndClassEx.hbrBackground = sys_color_brush(COLOR_BTNFACE);

	this->on_message(WM_SIZE, [this](wl::params p) -> LRESULT {
		if (p.wParam == SIZE_MINIMIZED) return 0;
		_client_w = LOWORD(p.lParam);
		_client_h = HIWORD(p.lParam);
		this->_layout();
		return 0;
		});

	this->on_message(WM_SETCURSOR, [this](wl::params p) -> LRESULT {
		HitZone z = _drag;
		if (z == HitZone::None) {
			POINT pt{};
			GetCursorPos(&pt);
			ScreenToClient(this->hwnd(), &pt);
			z = this->_hit_test(pt.x, pt.y);
		}
		if (z == HitZone::VBar) {
			SetCursor(LoadCursor(nullptr, IDC_SIZEWE));
			return TRUE;
		}
		if (z == HitZone::HBar) {
			SetCursor(LoadCursor(nullptr, IDC_SIZENS));
			return TRUE;
		}
		return DefWindowProc(this->hwnd(),
			p.message, p.wParam, p.lParam);
		});

	this->on_message(WM_LBUTTONDOWN, [this](wl::params p) -> LRESULT {
		const int x = GET_X_LPARAM(p.lParam);
		const int y = GET_Y_LPARAM(p.lParam);
		const HitZone z = this->_hit_test(x, y);
		if (z != HitZone::None) {
			_drag = z;
			SetCapture(this->hwnd());
		}
		return 0;
		});

	this->on_message(WM_MOUSEMOVE, [this](wl::params p) -> LRESULT {
		if (_drag == HitZone::None) return 0;
		const int x = GET_X_LPARAM(p.lParam);
		const int y = GET_Y_LPARAM(p.lParam);
		if (_drag == HitZone::VBar) {
			_leftWidth = std::clamp(x,
				MIN_LEFT,
				_client_w - MIN_CENTER - BAR_THICK);
		}
		else {
			const int newBottomH =
				_client_h - y - BAR_THICK;
			_bottomHeight = std::clamp(newBottomH,
				MIN_BOTTOM,
				_client_h - MIN_CENTER - BAR_THICK);
		}
		this->_layout();
		return 0;
		});

	this->on_message(WM_LBUTTONUP, [this](wl::params) -> LRESULT {
		if (_drag != HitZone::None) {
			_drag = HitZone::None;
			ReleaseCapture();
		}
		return 0;
		});
}

void dock_layout::set_left(HWND hwnd)
{
	_left = hwnd;
	if (hwnd) SetParent(hwnd, this->hwnd());
	this->_layout();
}

void dock_layout::set_center(HWND hwnd)
{
	_center = hwnd;
	if (hwnd) SetParent(hwnd, this->hwnd());
	this->_layout();
}

void dock_layout::set_bottom(HWND hwnd)
{
	_bottom = hwnd;
	if (hwnd) SetParent(hwnd, this->hwnd());
	this->_layout();
}

void dock_layout::set_left_width(int px)
{
	_leftWidth = px;
	this->_layout();
}

void dock_layout::set_bottom_height(int px)
{
	_bottomHeight = px;
	this->_layout();
}

void dock_layout::show_left(bool visible)
{
	_leftVisible = visible;
	if (_left) ShowWindow(_left, visible ? SW_SHOW : SW_HIDE);
	this->_layout();
}

void dock_layout::show_bottom(bool visible)
{
	_bottomVisible = visible;
	if (_bottom) ShowWindow(_bottom, visible ? SW_SHOW : SW_HIDE);
	this->_layout();
}


/*
int dock_layout::_clamp(int v, int lo, int hi) noexcept
{
	if (hi < lo) hi = lo;
	if (v < lo)  return lo;
	if (v > hi)  return hi;
	return v;
}
*/

dock_layout::HitZone dock_layout::_hit_test(int x, int y) const noexcept
{
	if (_client_w <= 0 || _client_h <= 0) return HitZone::None;
	const int leftW = _effective_left_width();
	const int bottomH = _effective_bottom_height();
	const int rightX = leftW + (_leftVisible ? BAR_THICK : 0);

	if (_leftVisible
		&& x >= leftW && x < leftW + BAR_THICK
		&& y >= 0 && y < _client_h)
	{
		return HitZone::VBar;
	}

	const int hbarY = _client_h - bottomH - BAR_THICK;
	if (_bottomVisible
		&& x >= rightX && x < _client_w
		&& y >= hbarY && y < hbarY + BAR_THICK)
	{
		return HitZone::HBar;
	}

	return HitZone::None;
}

int dock_layout::_effective_left_width() const noexcept
{
	if (!_leftVisible) return 0;
	const int max = _client_w - MIN_CENTER - BAR_THICK;
	return std::clamp(_leftWidth, MIN_LEFT, max < MIN_LEFT ? MIN_LEFT : max);
}

int dock_layout::_effective_bottom_height() const noexcept
{
	if (!_bottomVisible) return 0;
	const int max = _client_h - MIN_CENTER - BAR_THICK;
	return std::clamp(_bottomHeight, MIN_BOTTOM, max < MIN_BOTTOM ? MIN_BOTTOM : max);
}

void dock_layout::_layout() noexcept
{
	if (!this->hwnd() || _client_w <= 0 || _client_h <= 0) return;

	const int leftW = _effective_left_width();
	const int bottomH = _effective_bottom_height();
	const int vbarW = _leftVisible ? BAR_THICK : 0;
	const int hbarH = _bottomVisible ? BAR_THICK : 0;
	const int rightX = leftW + vbarW;
	const int rightW = _client_w - rightX;
	const int centerH = _client_h - bottomH - hbarH;
	const int bottomY = _client_h - bottomH;

	defer_window_pos dwp(3);

	if (_left && _leftVisible) {
		dwp.defer(_left, nullptr,
			0, 0, leftW, _client_h,
			SWP_NOZORDER | SWP_NOACTIVATE);
	}
	if (_center) {
		dwp.defer(_center, nullptr,
			rightX, 0, rightW, centerH,
			SWP_NOZORDER | SWP_NOACTIVATE);
	}
	if (_bottom && _bottomVisible) {
		dwp.defer(_bottom, nullptr,
			rightX, bottomY, rightW, bottomH,
			SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

} // namespace wlx
