#pragma once
#include <Windows.h>
#include <CommCtrl.h>
#include "winlamb/window_main.h"
#include "winlambxe/utils/splitter_horizontal.h"
#include "winlambxe/utils/splitter_vertical.h"

/**
 * @brief Root application window, rebuilt to test the vertical splitter.
 */
class SplitterTestWindow : public wl::window_main {
private:
    HWND _hEditLeft = nullptr;
    HWND _hEditRight = nullptr;
    wlx::splitter_horizontal _splitter;

    /**
     * @brief Repositions the splitter window natively triggering WM_SIZE cascade.
     */
    void _layout() noexcept;

public:
    SplitterTestWindow();

    SplitterTestWindow(SplitterTestWindow &&) = default;
    SplitterTestWindow &operator=(SplitterTestWindow &&) = default;
};
