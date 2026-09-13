/**
 * @file webview.h
 * @brief Mock browser-style panel built on wl::window_control.
 *
 * Follows the WinLamb move-only pattern.
 */

#pragma once
#include <Windows.h>
#include "winlamb/window_control.h"
#include "winlamb/textbox.h"
#include "winlamb/internals/tstring.h"

namespace wlx {

/**
 * @brief Mock browser-style embedded web panel.
 *
 * Displays a read-only URL bar and a placeholder content area. Provides the
 * structural API of an embedded browser — navigate(), get_url() — without
 * requiring an external browser runtime.
 *
 * @par Replacing with a real browser engine
 * Substitute the WM_CREATE body with one of the following:
 *
 * **Microsoft WebView2 (recommended):**
 * @code
 * #include <WebView2.h>
 * // In WM_CREATE: call CreateCoreWebView2EnvironmentWithOptions(...) then
 * // ICoreWebView2Environment::CreateCoreWebView2Controller(hwnd(), ...).
 * // See https://docs.microsoft.com/en-us/microsoft-edge/webview2/
 * @endcode
 *
 * **Legacy IWebBrowser2 (COM in-place activation):**
 * @code
 * #include <SHDocVw.h>   // IWebBrowser2
 * #include <AtlBase.h>   // ATL-based hosting via AtlAxCreateControl()
 * // Or implement IOleClientSite + IOleInPlaceSite manually and call
 * // CoCreateInstance(CLSID_WebBrowser) + IOleObject::DoVerb(OLEIVERB_INPLACEACTIVATE).
 * @endcode
 */
class webview : public wl::window_control {
private:
    wl::textbox _urlBar;
    wl::tstring _currentUrl;

    static constexpr int URL_BAR_HEIGHT = 26; ///< Height of the URL bar in pixels.
    static constexpr int IDC_URLBAR     = 1;  ///< Child ID for the URL textbox.

public:
    /**
     * @brief Constructs the webview and registers its message handlers.
     *
     * The unique window class name `WL_WEBVIEW` is set here so that
     * wl::window_control::create() can register it before the HWND is created.
     */
    webview();

    webview(webview&&) = default;
    webview& operator=(webview&&) = default; ///< Move-only.

    /**
     * @brief Navigates to the given URL, updating the URL bar display.
     *
     * In this placeholder implementation the URL is stored and displayed in the
     * read-only textbox. Replace with `pWebBrowser->Navigate(url, ...)` or the
     * WebView2 equivalent.
     *
     * @param url  Target URL (e.g. `_T("https://example.com")`).
     */
    webview& navigate(const wl::tstring& url);

    /**
     * @brief Returns the most recently navigated-to URL.
     */
    const wl::tstring& get_url() const noexcept;
};

} // namespace wlx
