/**
 * @file document_tabs.h
 * @brief Center-area document tab widget with closable tabs.
 *
 * Hosts N child HWNDs as switchable document pages. Tabs sit at the top of
 * the widget; each tab carries a close button (provided by wlx::tabctrl).
 * Closing a tab hides the page HWND and notifies the registered callback;
 * destruction of the page HWND remains the caller's responsibility.
 *
 * Differences from dock_window:
 *   - No dock chrome / title bar.
 *   - Close button on each tab is wired to actually remove the document.
 */

#pragma once
#include <functional>
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

    class document_tabs : public wl::window_control {
    public:
        static constexpr int IDC_TABS = 9201;

        document_tabs();

        document_tabs(document_tabs &&) = default;
        document_tabs &operator=(document_tabs &&) = default;

        document_tabs &add_document(const wl::tstring &label, HWND page);

        document_tabs &select_document(size_t index);

        int selected_index() const noexcept;
        size_t count() const noexcept;

        document_tabs &on_close_document(std::function<void(size_t, HWND)> cb);

    private:
        wlx::tabctrl _tabs;
        std::vector<HWND> _pages;
        int _activeIdx = -1;
        std::function<void(size_t, HWND)> _onClose;
        wl::font _font;
        int _tabH = 22;

        void _recompute_metrics() noexcept;

        void _close_document(size_t index);

        void _layout(int w, int h) noexcept;
    };

} // namespace wlx
