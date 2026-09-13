/**
 * @file richedit.h
 * @brief WinLamb wrapper for the modern RichEdit control (MSFTEDIT_CLASS / RICHEDIT50W).
 *
 * Follows the WinLamb move-only pattern.
 */

#pragma once
#include <string>
#include <stdexcept>
#include <tchar.h>
#include <Windows.h>
#include <Richedit.h>
#include "winlamb/internals/base_native_ctrl.h"
#include "winlamb/internals/base_native_ctrl_pubm.h"
#include "winlamb/internals/base_focus_pubm.h"
#include "winlamb/internals/styler.h"
#include "winlamb/wnd.h"

namespace wlx {

using tstring = std::basic_string<TCHAR>;

/**
 * @brief WinLamb wrapper for the modern RichEdit control (RICHEDIT50W).
 *
 * Loads `Msftedit.dll` (with `riched20.dll` as fallback) the first time a
 * richedit is created; subsequent calls reuse the already-loaded module.
 *
 * The control is created with `ES_MULTILINE`, `WS_VSCROLL`, `ES_AUTOVSCROLL`,
 * `ES_WANTRETURN`, and `WS_EX_CLIENTEDGE` by default.
 */
class richedit final :
    public wl::wnd,
    public wl::_wli::base_native_ctrl_pubm<richedit>,
    public wl::_wli::base_focus_pubm<richedit>
{
private:
    HWND                       _hWnd = nullptr;
    wl::_wli::base_native_ctrl _baseNativeCtrl{_hWnd};

    /**
     * @brief Loads the RichEdit DLL exactly once per process.
     * @throws std::runtime_error if neither Msftedit.dll nor riched20.dll can be loaded.
     */
    static HMODULE _ensure_loaded();

public:
    /** @brief Wraps window style changes via Get/SetWindowLongPtr. */
    wl::_wli::styler<richedit> style{this};

    richedit();

    richedit(richedit&&) = default;
    richedit& operator=(richedit&&) = default; ///< Move-only.

    /**
     * @brief Creates the RichEdit control as a child window.
     *
     * Loads `Msftedit.dll` before the CreateWindowEx call.
     *
     * @param hParent  Parent window handle.
     * @param ctrlId   Child-window control identifier.
     * @param pos      Top-left position in parent client coordinates.
     * @param size     Width and height of the control.
     */
    richedit& create(HWND hParent, int ctrlId, POINT pos, SIZE size);

    /** @overload Creates as a child of a wnd-derived parent. */
    richedit& create(const wl::wnd* parent, int ctrlId, POINT pos, SIZE size);

    /**
     * @brief Replaces all text in the control.
     * @param text  New content (TCHAR string; wide under UNICODE, ANSI otherwise).
     */
    richedit& set_text(const tstring& text) noexcept;

    /**
     * @brief Returns the full text content of the control.
     */
    tstring get_text() const;

    /**
     * @brief Appends @p text at the end of the current content.
     *
     * Uses EM_SETSEL(-1, -1) + EM_REPLACESEL to avoid clearing the undo buffer.
     *
     * @param text  TCHAR string to append.
     */
    richedit& append_text(const tstring& text) noexcept;

    /**
     * @brief Applies a font to all text in the control via CHARFORMAT2.
     *
     * Sets CFM_FACE and CFM_SIZE with SCF_ALL scope.
     *
     * @param face    Font face name (e.g. `_T("Consolas")`).
     * @param sizePt  Point size (e.g. `11`). Internally stored as half-points.
     */
    richedit& set_font(LPCTSTR face, int sizePt) noexcept;

    /**
     * @brief Toggles the read-only state of the control.
     * @param ro  `true` to make the control read-only; `false` to allow editing.
     */
    richedit& set_readonly(bool ro) noexcept;
};

} // namespace wlx
