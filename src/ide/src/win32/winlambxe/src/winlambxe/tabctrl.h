/**
 * @file tabctrl.h
 * @brief WinLamb wrapper for the Win32 tab control (WC_TABCONTROL).
 *
 * Follows the WinLamb move-only pattern.
 */

#pragma once
#include <functional>
#include <vector>
#include <stdexcept>
#include <Windows.h>
#include <CommCtrl.h>
#include <windowsx.h>
#include "winlamb/internals/base_native_ctrl.h"
#include "winlamb/internals/base_native_ctrl_pubm.h"
#include "winlamb/internals/base_focus_pubm.h"
#include "winlamb/internals/styler.h"
#include "winlamb/internals/tstring.h"
#include "winlamb/subclass.h"
#include "winlamb/wnd.h"

namespace wlx {

    /**
     * @brief WinLamb wrapper for the Win32 tab control (WC_TABCONTROL).
     *
     * Supports owner-drawn tabs with per-tab close buttons. The owning window
     * **must** forward its `WM_DRAWITEM` message to draw_item() for tabs to render.
     *
     * @par Minimal usage
     * @code
     * // Member:
     * wlx::tabctrl _tabs;
     *
     * // WM_CREATE:
     * _tabs.create(this, IDC_TABS, {0, 0}, {800, 600});
     * _tabs.add_tab(_T("Tab 1")).add_tab(_T("Tab 2"));
     * _tabs.on_close_tab([this](size_t i){ _tabs.remove_tab(i); });
     *
     * // WM_DRAWITEM:
     * const auto& dis = *reinterpret_cast<DRAWITEMSTRUCT*>(p.lParam);
     * if (dis.CtlID == IDC_TABS) { _tabs.draw_item(dis); return TRUE; }
     * @endcode
     */
    class tabctrl final : public wl::wnd, public wl::_wli::base_native_ctrl_pubm<tabctrl>, public wl::_wli::base_focus_pubm<tabctrl> {
    private:
        HWND _hWnd = nullptr;
        wl::_wli::base_native_ctrl _baseNativeCtrl{_hWnd};
        wl::subclass _subclass;
        std::vector<wl::tstring> _labels;
        std::function<void(size_t)> _onCloseTab;
        bool _closable = true;       ///< Whether tabs render and react to a close "X".
        int _hoveredCloseIdx = -1;   ///< Tab whose close button is currently hovered, or -1.
        bool _mouseTracking = false; ///< TrackMouseEvent (TME_LEAVE) currently armed.

        static constexpr int CLOSE_BTN_SIZE = 14;  ///< Close button side length in pixels.
        static constexpr int CLOSE_BTN_MARGIN = 8; ///< Gap between close button and tab right edge (and between label and close button).
        static constexpr int TAB_RIGHT_PAD = 12;   ///< Symmetric right-side breathing room when close buttons are disabled.

        /**
         * @brief Returns the close-button bounding rect for the given tab rect.
         * @param tabRc  Tab bounding rect (from TabCtrl_GetItemRect).
         */
        static RECT _close_btn_rect(const RECT &tabRc) noexcept;

        /**
         * @brief Hit-tests @p pt against every tab's close button.
         * @return Zero-based tab index, or -1 if no close button was hit (or close buttons are disabled).
         */
        int _hit_test_close(POINT pt) const noexcept;

        /**
         * @brief Returns @p label padded with trailing spaces so that its measured
         *        width is roughly @p extraPx pixels wider than the bare label.
         *        Used to grow the system-computed tab rect to make room for the
         *        close button (or symmetric padding when not closable).
         */
        static wl::tstring _pad_label(HWND hwnd, const wl::tstring &label, int extraPx);

        /**
         * @brief Re-applies the current padding policy to every existing tab's
         *        TCITEM text, so the Win32 tab control re-measures and resizes.
         */
        void _refresh_padding();

        /**
         * @brief Draws an "X" glyph (two diagonal GDI lines) into @p r using a
         *        DPI-scaled solid pen of @p stroke. Font-independent.
         */
        static void _draw_close_x(HDC hdc, const RECT &r, COLORREF stroke) noexcept;

    public:
        /** @brief Wraps window style changes via Get/SetWindowLongPtr. */
        wl::_wli::styler<tabctrl> style{this};

        /**
         * @brief Constructs the tabctrl and registers the subclass close-button handler.
         *
         * The WM_LBUTTONDOWN handler is registered here (before any HWND exists) so
         * that it is in place before the first message arrives after install_subclass().
         */
        tabctrl();

        tabctrl(tabctrl &&) = default;
        tabctrl &operator=(tabctrl &&) = default; ///< Move-only.

        /**
         * @brief Creates the tab control as a child window.
         * @param hParent  Parent window handle.
         * @param ctrlId   Child-window control identifier.
         * @param pos      Top-left position in parent client coordinates.
         * @param size     Width and height of the control.
         */
        tabctrl &create(HWND hParent, int ctrlId, POINT pos, SIZE size);

        /** @overload Creates as a child of a wnd-derived parent. */
        tabctrl &create(const wl::wnd *parent, int ctrlId, POINT pos, SIZE size);

        /**
         * @brief Appends a new tab with the given label.
         * @param label  Tab caption (null-terminated TCHAR string).
         */
        tabctrl &add_tab(const TCHAR *label);

        /** @overload */
        tabctrl &add_tab(const wl::tstring &label);

        /**
         * @brief Removes the tab at @p index.
         * @param index  Zero-based index of the tab to remove.
         */
        tabctrl &remove_tab(size_t index);

        /**
         * @brief Returns the zero-based index of the currently selected tab, or -1.
         */
        int selected_index() const noexcept;

        /**
         * @brief Programmatically selects the tab at @p index.
         * @param index  Zero-based tab index.
         */
        tabctrl &select(size_t index) noexcept;

        /**
         * @brief Returns the total number of tabs.
         */
        size_t count() const noexcept;

        /**
         * @brief Returns the tab content display area in the control's client coordinates.
         *
         * Tab page child windows should be mapped from this coordinate system to the
         * parent window's client coordinates using MapWindowPoints before positioning.
         *
         * @code
         * RECT area = _tabctrl.display_area();
         * MapWindowPoints(_tabctrl.hwnd(), hwnd(), (LPPOINT)&area, 2);
         * // area is now in main-window client coordinates
         * @endcode
         */
        RECT display_area() const noexcept;

        /**
         * @brief Registers a callback invoked when the user clicks a tab's close button.
         * @param fn  Callable receiving the zero-based index of the tab to close.
         *            Typically calls remove_tab() and updates page visibility.
         */
        tabctrl &on_close_tab(std::function<void(size_t)> fn);

        /**
         * @brief Enables or disables per-tab close buttons.
         *
         * When disabled, the "X" is not drawn, the right-side margin is not
         * reserved, mouse hover does not highlight, and clicks in the (former)
         * close-button area are passed through to the tab control.
         *
         * Safe to call before or after create(). When called after tabs already
         * exist, the per-tab text padding is recomputed and the control is
         * invalidated for repaint.
         *
         * @param yes  true (default) to keep close buttons; false to suppress them.
         */
        tabctrl &closable(bool yes) noexcept;

        /** @brief Returns whether close buttons are enabled. */
        bool closable() const noexcept;

        /**
         * @brief Draws a single owner-draw tab item.
         *
         * Must be called from the **parent window's** WM_DRAWITEM handler.
         * The parent receives WM_DRAWITEM because TCS_OWNERDRAWFIXED is set.
         *
         * @param dis  DRAWITEMSTRUCT delivered by WM_DRAWITEM.
         */
        void draw_item(const DRAWITEMSTRUCT &dis) noexcept;
    };

} // namespace wlx
