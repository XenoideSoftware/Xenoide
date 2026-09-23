/**
 * @file tab_page_richtext.h
 * @brief Tab page hosting a wlx::richedit control.
 *
 * Part of ModernWinApp.
 */

#pragma once
#include <Windows.h>
#include "winlamb/window_control.h"
#include "winlambxe/richedit.h"

/**
 * @brief Tab page that fills its entire client area with a RichEdit50W control.
 *
 * Pre-loaded with sample text demonstrating the richedit wrapper capabilities.
 */
class TabPageRichText : public wl::window_control {
private:
    wlx::richedit _edit;

public:
    /**
     * @brief Constructs the tab page and registers WM_CREATE / WM_SIZE handlers.
     */
    TabPageRichText();

    TabPageRichText(TabPageRichText &&) = default;
    TabPageRichText &operator=(TabPageRichText &&) = default; ///< Move-only.
};
