/**
 * @file toolbar.h
 * @brief WinLamb wrapper for the Win32 toolbar control (WC_TOOLBAR).
 *
 * Wraps the @c WC_TOOLBAR window class from @c Comctl32.dll. The wrapper is
 * move-only and follows the same pattern as @c richedit.h and @c tabctrl.h in
 * this folder.
 *
 * The parent window receives button clicks via @c WM_COMMAND (control id ==
 * the @c idCommand passed to @ref add_button / @ref insert_button). Dropdown
 * and custom-draw notifications arrive via @c WM_NOTIFY; the parent forwards
 * them to @ref process_notify which dispatches to the typed @c on_* callbacks.
 *
 * @par Minimal usage
 * @code
 * // Member:
 * wlx::toolbar _tb;
 *
 * // WM_CREATE:
 * _tb.create(this, IDC_TOOLBAR, {0, 0}, {0, 0})
 *     .add_button(ID_NEW,   STD_FILENEW,  _T("New"))
 *     .add_button(ID_OPEN,  STD_FILEOPEN, _T("Open"))
 *     .add_separator()
 *     .add_button(ID_SAVE,  STD_FILESAVE, _T("Save"))
 *     .auto_size();
 *
 * _tb.on_click([this](int id){ ... handle WM_COMMAND-style clicks ... });
 *
 * // WM_NOTIFY (in the parent):
 * auto* nmh = reinterpret_cast<NMHDR*>(p.lParam);
 * if (nmh->hwndFrom == _tb.hwnd()) {
 *     _tb.process_notify(*reinterpret_cast<NMHDR*>(nmh), idFrom);
 *     return 0;
 * }
 * @endcode
 */

#pragma once
#include <functional>
#include <stdexcept>
#include <vector>
#include <Windows.h>
#include <CommCtrl.h>
#include "winlamb/internals/base_native_ctrl.h"
#include "winlamb/internals/base_native_ctrl_pubm.h"
#include "winlamb/internals/base_focus_pubm.h"
#include "winlamb/internals/styler.h"
#include "winlamb/internals/tstring.h"
#include "winlamb/wnd.h"

namespace wlx {

    /**
     * @brief WinLamb wrapper for the Win32 toolbar control (@c WC_TOOLBAR).
     *
     * Calls @c InitCommonControlsEx with @c ICC_BAR_CLASSES the first time a
     * @c toolbar is created; subsequent calls reuse the already-initialised
     * common-controls state.
     *
     * @par Image lists
     * Standard image lists (@c IDB_STD_SMALL_COLOR and @c IDB_STD_LARGE_COLOR) are
     * installed automatically on @ref create. Use @ref set_image_list /
     * @ref set_disabled_image_list / @ref set_hot_image_list to install custom
     * image lists for application-supplied bitmaps.
     *
     * @par Command routing
     * Button clicks are emitted as @c WM_COMMAND messages carrying the
     * @c idCommand assigned at @ref add_button. Register @c on_click to receive a
     * typed callback instead of having to inspect @c WM_COMMAND in the parent.
     *
     * @par Flat vs flat-divider styles
     * Pass @c TBSTYLE_FLAT or @c TBSTYLE_FLAT | @c TBSTYLE_LIST through @ref create
     * as part of @p styles by setting them on @ref style after @ref create, e.g.
     * @code _tb.style.edit_style(TBSTYLE_FLAT, 0); @endcode
     */
    class toolbar final : public wl::wnd, public wl::_wli::base_native_ctrl_pubm<toolbar>, public wl::_wli::base_focus_pubm<toolbar> {
    private:
        HWND _hWnd = nullptr;
        wl::_wli::base_native_ctrl _baseNativeCtrl{_hWnd};

        // Notification callbacks (typed; only the ones the user installs are invoked).
        std::function<void(int)> _onClick;                              ///< WM_COMMAND click on a button (idCommand).
        std::function<void(int)> _onDropDown;                           ///< TBN_DROPDOWN (idCommand).
        std::function<void(int)> _onBeginDrag;                          ///< TBN_BEGINDRAG (idCommand).
        std::function<void(int)> _onEndDrag;                            ///< TBN_ENDDRAG (idCommand).
        std::function<void(int)> _onGetButtonInfo;                      ///< TBN_GETBUTTONINFO.
        std::function<void()> _onReset;                                 ///< TBN_RESET (rebar/chevron customisation).
        std::function<void()> _onQueryDelete;                           ///< TBN_QUERYDELETE.
        std::function<void()> _onQueryInsert;                           ///< TBN_QUERYINSERT.
        std::function<void(int, const NMTBCUSTOMDRAW *)> _onCustomDraw; ///< NM_CUSTOMDRAW per-stage callback.
        std::function<void()> _onRClick;                                ///< TBN_TOOLBARCLICK / NM_RCLICK.
        std::function<void(const NMHDR &)> _onUnhandledNotify;          ///< fallback.

        /**
         * @brief Initialises @c Comctl32.dll common controls exactly once per process.
         *
         * Calls @c InitCommonControlsEx with @c ICC_BAR_CLASSES. Safe to invoke
         * multiple times; the second and subsequent calls are no-ops.
         */
        static void _ensure_loaded() noexcept;

    public:
        /** @brief Wraps window style changes via Get/SetWindowLongPtr. */
        wl::_wli::styler<toolbar> style{this};

        toolbar();

        toolbar(toolbar &&) = default;
        toolbar &operator=(toolbar &&) = default; ///< Move-only.

        // ====================================================================
        // Creation
        // ====================================================================

        /**
         * @brief Creates the toolbar control as a child window.
         *
         * Combined styles default to @c WS_CHILD | @c WS_VISIBLE | @c TBSTYLE_TOOLTIPS
         * | @c CCS_NODIVIDER | @c CCS_NOPARENTALIGN | @c CCS_NORESIZE so the parent
         * owns layout. @c TBSTYLE_FLAT may be added via @ref style afterwards.
         *
         * @param hParent  Parent window handle.
         * @param ctrlId   Child-window control identifier.
         * @param pos      Top-left position in parent client coordinates.
         * @param size     Width and height of the control (often @c {0,0}; the
         *                 toolbar auto-sizes to its content).
         */
        toolbar &create(HWND hParent, int ctrlId, POINT pos, SIZE size);

        /** @overload Creates as a child of a wnd-derived parent. */
        toolbar &create(const wl::wnd *parent, int ctrlId, POINT pos, SIZE size);

        // ====================================================================
        // Buttons
        // ====================================================================

        /**
         * @brief Appends a standard-image button.
         *
         * @param idCommand  Command id sent in @c WM_COMMAND on click (use 0 for a
         *                   placeholder; @c -1 not allowed).
         * @param stdBitmap  Index in @c IDB_STD_SMALL_COLOR (e.g. @c STD_FILENEW).
         * @param label      Tooltip / display string (may be empty).
         * @param styles     Per-button @c BTNS_* flags (e.g. @c BTNS_AUTOSIZE,
         *                   @c BTNS_BUTTON, @c BTNS_CHECK).
         * @param state      Initial @c TBSTATE_* flags (@c TBSTATE_ENABLED by default).
         */
        toolbar &add_button(int idCommand, int stdBitmap, const TCHAR *label, BYTE styles = BTNS_BUTTON, BYTE state = TBSTATE_ENABLED);

        /** @overload Accepts an @c wl::tstring label. */
        toolbar &add_button(int idCommand, int stdBitmap, const wl::tstring &label, BYTE styles = BTNS_BUTTON, BYTE state = TBSTATE_ENABLED);

        /**
         * @brief Appends a button that uses a bitmap from the user image list.
         *
         * Requires a user image list installed via @ref set_image_list. The
         * bitmap index is zero-based into that image list.
         */
        toolbar &add_button_bitmap(int idCommand, int bitmapIndex, const TCHAR *label, BYTE styles = BTNS_BUTTON, BYTE state = TBSTATE_ENABLED);

        /**
         * @brief Appends a separator (a thin gap or vertical bar).
         * @param width  Width in pixels (0 lets the system pick a sensible gap).
         */
        toolbar &add_separator(int width = 0);

        /**
         * @brief Inserts a button at @p index (0-based).
         *
         * @param index       Insertion position; @c -1 (default) appends.
         * @param idCommand   Command id (0 for separators).
         * @param stdBitmap   Index in standard image list or user image list.
         * @param label       Display / tooltip string.
         * @param styles      @c BTNS_* flags.
         * @param state       @c TBSTATE_* flags.
         */
        toolbar &insert_button(int index, int idCommand, int stdBitmap, const TCHAR *label, BYTE styles = BTNS_BUTTON, BYTE state = TBSTATE_ENABLED);

        /** @brief Removes the button whose command id matches @p idCommand. */
        toolbar &delete_button(int idCommand) noexcept;

        /**
         * @brief Returns the zero-based index of the button whose command id is
         *        @p idCommand, or @c -1 if not found.
         */
        int button_index(int idCommand) const noexcept;

        /** @brief Returns the number of buttons currently in the toolbar. */
        int button_count() const noexcept;

        /** @brief Returns full @c TBBUTTON info for the button at @p index. */
        TBBUTTON button_info(int index) const;

        /**
         * @brief Sets the command id and/or bitmap of the button at @p index.
         * @param index  0-based button index.
         * @param cmd    New command id, or @c -1 to keep current.
         * @param bitmap New image-list/std-bitmap index, or @c -1 to keep current.
         */
        toolbar &set_button_info(int index, int cmd, int bitmap) noexcept;

        // ====================================================================
        // Button state
        // ====================================================================

        /** @brief Enables or disables the button with @p idCommand. */
        toolbar &enable_button(int idCommand, bool enable) noexcept;

        /** @brief Checks or unchecks a button (use with @c BTNS_CHECK / @c BTNS_CHECKGROUP). */
        toolbar &check_button(int idCommand, bool check) noexcept;

        /** @brief Presses or releases a button visually. */
        toolbar &press_button(int idCommand, bool press) noexcept;

        /** @brief Hides or shows the button with @p idCommand. */
        toolbar &hide_button(int idCommand, bool hide) noexcept;

        /** @brief Returns the @c TBSTATE_* flags of @p idCommand, or 0 if absent. */
        BYTE button_state(int idCommand) const noexcept;

        // ====================================================================
        // Image lists
        // ====================================================================

        /** @brief Installs the normal-state image list (HIMAGELIST). */
        toolbar &set_image_list(HIMAGELIST himl) noexcept;

        /** @brief Installs the disabled-state image list (HIMAGELIST). */
        toolbar &set_disabled_image_list(HIMAGELIST himl) noexcept;

        /** @brief Installs the hot-track image list (HIMAGELIST). */
        toolbar &set_hot_image_list(HIMAGELIST himl) noexcept;

        /** @brief Adds @p n system-standard bitmaps (e.g. @c IDB_STD_SMALL_COLOR). */
        toolbar &add_std_bitmaps(int resId, int n) noexcept;

        /**
         * @brief Adds @p n bitmaps from a user-supplied @c HBITMAP (@c 24bpp or less).
         *
         * The toolbar takes ownership of palette translation but not of @p hBmp;
         * the caller may free it once the call returns.
         */
        toolbar &add_user_bitmaps(HBITMAP hBmp, int n) noexcept;

        // ====================================================================
        // Sizing & layout
        // ====================================================================

        /**
         * @brief Sets the size of buttons in pixels.
         *
         * @param width  Button width (cx).
         * @param height Button height (cy).
         */
        toolbar &set_button_size(int width, int height) noexcept;

        /** @brief Sets the padding between button bitmaps and button edges. */
        toolbar &set_padding(int cx, int cy) noexcept;

        /** @brief Asks the toolbar to resize itself to fit its contents. */
        toolbar &auto_size() noexcept;

        /** @brief Returns the toolbar's preferred height in pixels. */
        int height() const noexcept;

        /** @brief Returns the toolbar's preferred width in pixels. */
        int width() const noexcept;

        /**
         * @brief Sets extended toolbar styles (@c TBSTYLE_EX_*).
         * @param mask  Bits to affect.
         * @param style Bits to set within @p mask (use 0 to clear).
         */
        toolbar &set_extended_style(DWORD mask, DWORD style) noexcept;

        /** @brief Returns the current extended styles (@c TBSTYLE_EX_*). */
        DWORD extended_style() const noexcept;

        // ====================================================================
        // Tooltips
        // ====================================================================

        /** @brief Returns the embedded tooltip control's HWND, or @c nullptr. */
        HWND tooltip_hwnd() const noexcept;

        /** @brief Sets the tooltip text for the button with @p idCommand. */
        toolbar &set_tooltip(int idCommand, const TCHAR *text) noexcept;

        /** @brief Max tip width in pixels (wraps long tips). 0 restores default. */
        toolbar &set_tooltip_max_width(int pixels) noexcept;
    };

} // namespace wlx