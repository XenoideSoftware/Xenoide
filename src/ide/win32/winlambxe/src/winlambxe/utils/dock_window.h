/**
 * @file dock_window.h
 * @brief Single dockable container with title bar and tabbed pages.
 *
 * Hosts N child HWNDs as switchable pages. Visual chrome is a GDI-painted
 * title band at the top and an owner-drawn tab strip at the bottom (Visual
 * Studio dock-pane convention).
 */

#pragma once
#include <utility>
#include <vector>
#include <Windows.h>
#include <CommCtrl.h>
#include "winlamb/window_control.h"
#include "winlamb/font.h"
#include "winlamb/internals/tstring.h"
#include "winlambxe/tabctrl.h"
#include "winlambxe/utils/defer_window_pos.h"

namespace wlx {

    class dock_window : public wl::window_control {
    public:
        static constexpr int IDC_TABS = 9001;

        dock_window();

        dock_window(dock_window &&) = default;
        dock_window &operator=(dock_window &&) = default;

        dock_window &set_title(wl::tstring text);

        dock_window &add_page(const wl::tstring &label, HWND page);

        dock_window &select_page(size_t index);

        int selected_index() const noexcept;

    private:
        wl::tstring _title;
        wlx::tabctrl _tabs;
        std::vector<HWND> _pages;
        int _activeIdx = -1;
        wl::font _font;
        int _titleH = 22;
        int _tabH = 22;

        void _recompute_metrics() noexcept;

        void _layout(int w, int h) noexcept;

        void _paint_title(HDC hdc, const RECT &rc) noexcept;
    };

} // namespace wlx
