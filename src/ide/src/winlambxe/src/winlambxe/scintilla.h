/**
 * @file scintilla.h
 * @brief WinLamb wrapper for the Scintilla 3 editing control.
 *
 * Wraps the @c "Scintilla" window class shipped by @c Scintilla.dll
 * (Scintilla 3.x). The wrapper is header-only, move-only, and follows the
 * same pattern as @c richedit.h and @c tabctrl.h in this folder.
 *
 * Notifications arrive at the parent window via @c WM_NOTIFY. The parent
 * forwards them to @ref wlx::scintilla::process_notify, which dispatches
 * to the typed @c on_* callbacks registered on the wrapper.
 *
 * @par Minimal usage
 * @code
 * // Member:
 * wlx::scintilla _editor;
 *
 * // WM_CREATE:
 * _editor.create(this, IDC_EDITOR, {0, 0}, {800, 600})
 *        .set_lexer_language("cpp")
 *        .show_line_numbers(true)
 *        .style_set_font(STYLE_DEFAULT, "Consolas")
 *        .style_set_size(STYLE_DEFAULT, 10)
 *        .style_clear_all()
 *        .set_use_tabs(false)
 *        .set_tab_width(4);
 *
 * _editor.on_modified([this](const SCNotification& n){ ... });
 *
 * // WM_NOTIFY (in the parent):
 * auto* nmh = reinterpret_cast<NMHDR*>(p.lParam);
 * if (nmh->hwndFrom == _editor.hwnd()) {
 *     _editor.process_notify(*reinterpret_cast<SCNotification*>(nmh));
 *     return 0;
 * }
 * @endcode
 */

#pragma once
#include <functional>
#include <stdexcept>
#include <string>
#include <Windows.h>
#include <Scintilla.h>
#include <SciLexer.h>
#include "winlamb/internals/base_native_ctrl.h"
#include "winlamb/internals/base_native_ctrl_pubm.h"
#include "winlamb/internals/base_focus_pubm.h"
#include "winlamb/internals/styler.h"
#include "winlamb/wnd.h"

namespace wlx {

    /**
     * @brief WinLamb wrapper for the Scintilla 3 editing control.
     *
     * Loads @c Scintilla.dll the first time a @c scintilla is created;
     * subsequent calls reuse the already-loaded module. After @ref create the
     * wrapper caches the direct-call function pointer (@c SCI_GETDIRECTFUNCTION /
     * @c SCI_GETDIRECTPOINTER) and uses it for every subsequent operation,
     * bypassing @c SendMessage for performance.
     *
     * Text I/O uses UTF-8 @c std::string, matching Scintilla's native buffer model.
     */
    class scintilla final : public wl::wnd, public wl::_wli::base_native_ctrl_pubm<scintilla>, public wl::_wli::base_focus_pubm<scintilla> {
    private:
        HWND _hWnd = nullptr;
        wl::_wli::base_native_ctrl _baseNativeCtrl{_hWnd};

        SciFnDirect _fnDirect = nullptr; ///< Cached @c SCI_GETDIRECTFUNCTION result.
        sptr_t _ptrDirect = 0;           ///< Cached @c SCI_GETDIRECTPOINTER result.

        // Notification callbacks (typed; only the ones the user installs are invoked).
        std::function<void(const SCNotification &)> _onModified;
        std::function<void()> _onSavePointReached;
        std::function<void()> _onSavePointLeft;
        std::function<void(int)> _onCharAdded;
        std::function<void(int)> _onUpdateUi;
        std::function<void(intptr_t, int, int)> _onMarginClick;
        std::function<void(intptr_t, intptr_t, int)> _onDoubleClick;
        std::function<void()> _onZoom;
        std::function<void()> _onFocusIn;
        std::function<void()> _onFocusOut;
        std::function<void(int)> _onKey;
        std::function<void(const SCNotification &)> _onUnhandledNotify;

        /**
         * @brief Loads @c Scintilla.dll exactly once per process.
         * @throws std::runtime_error if @c Scintilla.dll cannot be loaded.
         */
        static HMODULE _ensure_loaded();

    public:
        /** @brief Wraps window style changes via Get/SetWindowLongPtr. */
        wl::_wli::styler<scintilla> style{this};

        scintilla();

        scintilla(scintilla &&) = default;
        scintilla &operator=(scintilla &&) = default; ///< Move-only.

        // ====================================================================
        // Creation
        // ====================================================================

        /**
         * @brief Creates the Scintilla control as a child window.
         *
         * Loads @c Scintilla.dll before the @c CreateWindowEx call and caches the
         * direct-call function pointer afterwards.
         *
         * @param hParent  Parent window handle.
         * @param ctrlId   Child-window control identifier.
         * @param pos      Top-left position in parent client coordinates.
         * @param size     Width and height of the control.
         */
        scintilla &create(HWND hParent, int ctrlId, POINT pos, SIZE size);

        /** @overload Creates as a child of a wnd-derived parent. */
        scintilla &create(const wl::wnd *parent, int ctrlId, POINT pos, SIZE size);

        sptr_t get_direct_pointer() const {
            return send(SCI_GETDIRECTPOINTER);
        }

        SciFnDirect get_direct_function() const {
            return reinterpret_cast<SciFnDirect>(send(SCI_GETDIRECTFUNCTION));
        }

    private:
        // ====================================================================
        // Raw send (escape hatch + foundation for typed methods)
        // ====================================================================

        /**
         * @brief Sends a Scintilla message, using the cached direct-call path.
         *
         * Falls back to @c SendMessageW if invoked before @ref create. All typed
         * methods on this wrapper route through @ref send.
         *
         * @param msg  @c SCI_* message identifier.
         * @param wp   @c WPARAM-equivalent.
         * @param lp   @c LPARAM-equivalent.
         * @return Whatever the @c SCI_* message returns (often a position, a
         *         length, or 0).
         */
        sptr_t send(unsigned msg, uptr_t wp = 0, sptr_t lp = 0) const noexcept;

    public:
        // ====================================================================
        // Text I/O (UTF-8)
        // ====================================================================

        /** @brief Replaces the entire buffer with @p text. */
        scintilla &set_text(const std::string &text);

        /** @brief Returns the entire buffer as a UTF-8 string. */
        std::string get_text() const;

        /** @brief Appends @p text at the end of the buffer. */
        scintilla &append_text(const std::string &text);

        /**
         * @brief Inserts @p text at byte offset @p pos.
         * @param pos   UTF-8 byte offset; @c -1 inserts at the current caret.
         * @param text  UTF-8 text to insert.
         */
        scintilla &insert_text(intptr_t pos, const std::string &text);

        /** @brief Deletes @p length bytes starting at @p pos. */
        scintilla &delete_range(intptr_t pos, intptr_t length);

        /** @brief Empties the entire buffer. */
        scintilla &clear_all() noexcept;

        /** @brief Total number of bytes in the buffer (excluding implicit NUL). */
        intptr_t length() const noexcept;

        /** @brief Returns the byte range @p [start, end) as a UTF-8 string. */
        std::string text_range(intptr_t start, intptr_t end) const;

        /** @brief Returns the byte at @p pos, or @c 0 if @p pos is out of range. */
        int char_at(intptr_t pos) const noexcept;

        // ====================================================================
        // Lines
        // ====================================================================

        /** @brief Total number of lines in the buffer (always @c >= 1). */
        intptr_t line_count() const noexcept;

        /** @brief Returns the zero-based line number containing @p pos. */
        intptr_t line_from_position(intptr_t pos) const noexcept;

        /** @brief Returns the byte offset of the start of @p line. */
        intptr_t position_from_line(intptr_t line) const noexcept;

        /** @brief Returns the byte length of @p line including line ending. */
        intptr_t line_length(intptr_t line) const noexcept;

        /** @brief Returns the text of @p line including the line terminator. */
        std::string line_text(intptr_t line) const;

        /** @brief Scrolls and moves the caret to the start of @p line. */
        scintilla &goto_line(intptr_t line) noexcept;

        /** @brief Scrolls and moves the caret to byte offset @p pos. */
        scintilla &goto_pos(intptr_t pos) noexcept;

        // ====================================================================
        // Selection / caret
        // ====================================================================

        /** @brief Sets the selection to @p [anchor, caret). */
        scintilla &set_selection(intptr_t anchor, intptr_t caret) noexcept;

        /** @brief Sets the selection anchor without moving the caret. */
        scintilla &set_anchor(intptr_t pos) noexcept;

        /** @brief Moves the caret without changing the anchor. */
        scintilla &set_current_pos(intptr_t pos) noexcept;

        /** @brief Returns the lower bound of the current selection. */
        intptr_t selection_start() const noexcept;

        /** @brief Returns the upper bound of the current selection. */
        intptr_t selection_end() const noexcept;

        /** @brief Returns the current caret byte offset. */
        intptr_t current_pos() const noexcept;

        /** @brief Returns the current selection anchor byte offset. */
        intptr_t anchor() const noexcept;

        /** @brief Returns the currently selected text as UTF-8. */
        std::string selected_text() const;

        /** @brief Selects the entire buffer. */
        scintilla &select_all() noexcept;

        /** @brief Replaces the current selection with @p text. */
        scintilla &replace_selection(const std::string &text);

        // ====================================================================
        // Editing state, undo/redo
        // ====================================================================

        /** @brief Sets the read-only flag. */
        scintilla &set_readonly(bool ro) noexcept;

        /** @brief Returns @c true if the control is read-only. */
        bool is_readonly() const noexcept;

        /** @brief Returns @c true if the buffer has been modified since the last save point. */
        bool is_modified() const noexcept;

        /** @brief Tells Scintilla the current state is the "clean" save point. */
        scintilla &set_save_point() noexcept;

        bool can_undo() const noexcept;
        bool can_redo() const noexcept;
        scintilla &undo() noexcept;
        scintilla &redo() noexcept;
        scintilla &begin_undo_action() noexcept;
        scintilla &end_undo_action() noexcept;
        scintilla &empty_undo_buffer() noexcept;

        // ====================================================================
        // Styling
        // ====================================================================

        /** @brief Resets every style to a copy of @c STYLE_DEFAULT. */
        scintilla &style_clear_all() noexcept;

        scintilla &style_set_font(int styleIndex, const char *face);
        scintilla &style_set_size(int styleIndex, int sizePt) noexcept;
        scintilla &style_set_fore(int styleIndex, COLORREF colour) noexcept;
        scintilla &style_set_back(int styleIndex, COLORREF colour) noexcept;
        scintilla &style_set_bold(int styleIndex, bool on) noexcept;
        scintilla &style_set_italic(int styleIndex, bool on) noexcept;
        scintilla &style_set_underline(int styleIndex, bool on) noexcept;
        scintilla &style_set_eol_filled(int styleIndex, bool on) noexcept;

        /**
         * @brief Selects a built-in lexer by name (e.g. @c "cpp", @c "python").
         *
         * Scintilla 3 ships its lexers in the same DLL; no Lexilla is required.
         */
        scintilla &set_lexer_language(const char *name);

        /** @brief Re-runs the lexer over the byte range @p [start, end). */
        scintilla &colourise(intptr_t start, intptr_t end) noexcept;

        // ====================================================================
        // Margins
        // ====================================================================

        scintilla &set_margin_type(int margin, int type) noexcept;
        scintilla &set_margin_width(int margin, int pixels) noexcept;
        scintilla &set_margin_mask(int margin, int mask) noexcept;
        scintilla &set_margin_sensitive(int margin, bool sensitive) noexcept;

        /**
         * @brief Convenience: enables or disables margin 0 as a line-number margin.
         * @param show   @c true to show, @c false to hide.
         * @param width  Width in pixels when shown (default @c 32).
         */
        scintilla &show_line_numbers(bool show, int width = 32) noexcept;

        // ====================================================================
        // Markers
        // ====================================================================

        scintilla &marker_define(int markerNum, int symbol) noexcept;
        scintilla &marker_set_fore(int markerNum, COLORREF colour) noexcept;
        scintilla &marker_set_back(int markerNum, COLORREF colour) noexcept;
        /** @brief Adds @p markerNum to @p line. Returns the marker handle. */
        int marker_add(intptr_t line, int markerNum) noexcept;
        scintilla &marker_delete(intptr_t line, int markerNum) noexcept;

        // ====================================================================
        // Indicators
        // ====================================================================

        scintilla &indicator_set_style(int indicator, int style) noexcept;
        scintilla &indicator_set_fore(int indicator, COLORREF colour) noexcept;
        /** @brief Selects @p indicator as the target for fill/clear range. */
        scintilla &set_current_indicator(int indicator) noexcept;
        scintilla &indicator_fill_range(intptr_t pos, intptr_t length) noexcept;
        scintilla &indicator_clear_range(intptr_t pos, intptr_t length) noexcept;

        // ====================================================================
        // Folding
        // ====================================================================

        /**
         * @brief Enables a typical folding setup.
         *
         * Sets fold flags, declares margin 2 as a sensitive symbol margin with
         * the standard fold-marker mask, and defines the standard fold-marker
         * symbols. The caller is still responsible for handling
         * @c SCN_MARGINCLICK (via @ref on_margin_click) to toggle folds.
         */
        scintilla &set_folding_enabled(bool on) noexcept;

        /**
         * @brief Toggles or sets the fold state of @p line.
         * @param action  One of @c SC_FOLDACTION_CONTRACT, @c SC_FOLDACTION_EXPAND,
         *                @c SC_FOLDACTION_TOGGLE.
         */
        scintilla &fold_line(intptr_t line, int action) noexcept;
        /** @brief Applies @p action to every fold in the buffer. */
        scintilla &fold_all(int action) noexcept;

        // ====================================================================
        // Search
        // ====================================================================

        scintilla &set_target_range(intptr_t start, intptr_t end) noexcept;
        scintilla &set_search_flags(int flags) noexcept;
        /**
         * @brief Searches for @p text in the current target range.
         * @return Byte offset of the match, or @c -1 if not found.
         */
        intptr_t search_in_target(const std::string &text) noexcept;
        /**
         * @brief Replaces the current target range with @p text.
         * @return Length of the replacement.
         */
        intptr_t replace_target(const std::string &text) noexcept;
        /**
         * @brief Find @p text in the byte range @p [start, end).
         * @return Byte offset of the match, or @c -1 if not found.
         */
        intptr_t find_text(int flags, intptr_t start, intptr_t end, const std::string &text);

        // ====================================================================
        // View configuration
        // ====================================================================

        scintilla &set_tab_width(int width) noexcept;
        scintilla &set_use_tabs(bool on) noexcept;
        scintilla &set_indent(int width) noexcept;
        scintilla &set_eol_mode(int mode) noexcept;
        scintilla &convert_eols(int mode) noexcept;
        scintilla &set_view_eol(bool on) noexcept;
        scintilla &set_view_whitespace(int mode) noexcept;
        scintilla &set_word_wrap(int mode) noexcept;
        scintilla &set_caret_line_visible(bool on) noexcept;
        scintilla &set_caret_line_back(COLORREF c) noexcept;
        scintilla &set_zoom(int level) noexcept;
        scintilla &zoom_in() noexcept;
        scintilla &zoom_out() noexcept;

        // ====================================================================
        // Notifications
        // ====================================================================

        /**
         * @brief Dispatches a Scintilla notification to the registered callbacks.
         *
         * Must be called from the **parent window's** @c WM_NOTIFY handler when
         * @c NMHDR::hwndFrom matches @ref hwnd.
         *
         * @param n  The full @c SCNotification payload received via @c WM_NOTIFY.
         */
        void process_notify(const SCNotification &n) noexcept;

        /** @brief Installs a callback for @c SCN_MODIFIED. */
        scintilla &on_modified(std::function<void(const SCNotification &)> fn);
        /** @brief Installs a callback for @c SCN_SAVEPOINTREACHED. */
        scintilla &on_save_point_reached(std::function<void()> fn);
        /** @brief Installs a callback for @c SCN_SAVEPOINTLEFT. */
        scintilla &on_save_point_left(std::function<void()> fn);
        /** @brief Installs a callback for @c SCN_CHARADDED (char passed as int). */
        scintilla &on_char_added(std::function<void(int)> fn);
        /** @brief Installs a callback for @c SCN_UPDATEUI (flags from @c n.updated). */
        scintilla &on_update_ui(std::function<void(int)> fn);
        /** @brief Installs a callback for @c SCN_MARGINCLICK (pos, margin, modifiers). */
        scintilla &on_margin_click(std::function<void(intptr_t, int, int)> fn);
        /** @brief Installs a callback for @c SCN_DOUBLECLICK (pos, line, modifiers). */
        scintilla &on_double_click(std::function<void(intptr_t, intptr_t, int)> fn);
        /** @brief Installs a callback for @c SCN_ZOOM. */
        scintilla &on_zoom(std::function<void()> fn);
        /** @brief Installs a callback for @c SCN_FOCUSIN. */
        scintilla &on_focus_in(std::function<void()> fn);
        /** @brief Installs a callback for @c SCN_FOCUSOUT. */
        scintilla &on_focus_out(std::function<void()> fn);
        /** @brief Installs a callback for @c SCN_KEY (key code in @c n.ch). */
        scintilla &on_key(std::function<void(int)> fn);
        /**
         * @brief Fallback callback invoked for any @c SCN_* not covered by a typed setter.
         *
         * Lets callers handle the long tail of Scintilla notifications without
         * extending this header.
         */
        scintilla &on_unhandled_notify(std::function<void(const SCNotification &)> fn);
    };

} // namespace wlx
