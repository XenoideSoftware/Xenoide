#include "winlambxe/utils/defer_window_pos.h"

namespace wlx {

defer_window_pos& defer_window_pos::operator=(defer_window_pos&& other) noexcept
{
	if (this != &other) {
		if (_hdwp) {
			EndDeferWindowPos(_hdwp);
		}
		_hdwp = other._hdwp;
		other._hdwp = nullptr;
	}
	return *this;
}

defer_window_pos::defer_window_pos(defer_window_pos&& other) noexcept : _hdwp(other._hdwp)
{
	other._hdwp = nullptr;
}

defer_window_pos::defer_window_pos(int num_windows /*= 1*/) noexcept : _hdwp(BeginDeferWindowPos(num_windows))
{

}

defer_window_pos::~defer_window_pos()
{
	if (_hdwp) {
		EndDeferWindowPos(_hdwp);
	}
}

bool defer_window_pos::defer(HWND hwnd, HWND hwndInsertAfter, int x, int y, int cx, int cy, UINT flags) noexcept
{
	if (!_hdwp) return false;
	HDWP new_hdwp = DeferWindowPos(_hdwp, hwnd, hwndInsertAfter, x, y, cx, cy, flags);
	if (new_hdwp) {
		_hdwp = new_hdwp;
		return true;
	}
	return false;
}

} // namespace wlx
