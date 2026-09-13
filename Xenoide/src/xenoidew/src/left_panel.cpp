#include "left_panel.h"
#include <commctrl.h>
#include <tchar.h>
#include <winlambxe/utils/sys_brush.h>


LeftPanel::LeftPanel()
{
	this->setup.wndClassEx.lpszClassName = _T("WL_LEFTPANEL");
	this->setup.wndClassEx.hbrBackground = wlx::sys_color_brush(COLOR_BTNFACE);

	this->on_message(WM_CREATE, [this](wl::params) -> LRESULT {
		RECT rc{};
		GetClientRect(this->hwnd(), &rc);
		this->_treeview.create(this->hwnd(), IDC_TREEVIEW_INNER,
			{ 0, 0 }, { rc.right, rc.bottom });

		if (this->_pendingImageList) {
			TreeView_SetImageList(this->_treeview.hwnd(),
				this->_pendingImageList, TVSIL_NORMAL);
		}

		// --- Sample project tree ---
		auto project = this->_treeview.items.add_root(_T("Project"));
		project.add_child(_T("src"));
		project.add_child(_T("include"));
		project.add_child(_T("docs"));

		auto deps = this->_treeview.items.add_root(_T("Dependencies"));
		deps.add_child(_T("winlamb"));
		deps.add_child(_T("mctrl"));
		deps.add_child(_T("HexCtrl"));
		return 0;
		});

	this->on_message(WM_SIZE, [this](wl::params p) -> LRESULT {
		const int cx = LOWORD(p.lParam);
		const int cy = HIWORD(p.lParam);
		if (this->_treeview.hwnd()) {
			SetWindowPos(this->_treeview.hwnd(), nullptr,
				0, 0, cx, cy,
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
		return 0;
		});
}

void LeftPanel::set_tree_image_list(HIMAGELIST il) noexcept {
	this->_pendingImageList = il;
	if (this->_treeview.hwnd()) {
		TreeView_SetImageList(this->_treeview.hwnd(), il, TVSIL_NORMAL);
	}
}
