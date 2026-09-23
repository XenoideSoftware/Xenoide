/**
 * @file dock_layout.h
 * @brief Three-zone dock composition (left | center / bottom) with self-
 *        contained drag bars.
 *
 * dock_layout positions three child HWNDs (left, center, bottom) directly
 * in its WM_SIZE, leaving two gaps between them that act as drag bars:
 * a full-height vertical bar between left and the right subarea, and a
 * horizontal bar across the right subarea between center and bottom.
 *
 * The bars are not separate child windows — they are uncovered regions of
 * dock_layout's own client area, painted by the class background brush
 * (COLOR_BTNFACE). dock_layout hit-tests its own WM_SETCURSOR /
 * WM_LBUTTONDOWN / WM_MOUSEMOVE / WM_LBUTTONUP to drive resizing.
 */

#pragma once
#include <Windows.h>
#include <WindowsX.h>
#include "winlamb/window_control.h"
#include "winlambxe/utils/defer_window_pos.h"

namespace wlx {

    class dock_layout : public wl::window_control {
    public:
        static constexpr int BAR_THICK = 6;
        static constexpr int MIN_LEFT = 60;
        static constexpr int MIN_CENTER = 100;
        static constexpr int MIN_BOTTOM = 60;

        dock_layout();

        dock_layout(dock_layout &&) = default;
        dock_layout &operator=(dock_layout &&) = default;

        void set_left(HWND hwnd);

        void set_center(HWND hwnd);

        void set_bottom(HWND hwnd);

        void set_left_width(int px);

        void set_bottom_height(int px);

        void show_left(bool visible);

        void show_bottom(bool visible);

    private:
        enum class HitZone { None, VBar, HBar };

        HWND _left = nullptr;
        HWND _center = nullptr;
        HWND _bottom = nullptr;
        int _leftWidth = 240;
        int _bottomHeight = 160;
        int _client_w = 0;
        int _client_h = 0;
        bool _leftVisible = true;
        bool _bottomVisible = true;
        HitZone _drag = HitZone::None;

        HitZone _hit_test(int x, int y) const noexcept;

        int _effective_left_width() const noexcept;

        int _effective_bottom_height() const noexcept;

        void _layout() noexcept;
    };

} // namespace wlx
