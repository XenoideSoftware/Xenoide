#pragma once
#include <Windows.h>

namespace wlx {

    /**
     * @brief Returns an HBRUSH-typed handle that refers to a Win32 system color.
     *
     * Wraps the standard Win32 idiom of casting `COLOR_xxx + 1` to HBRUSH.
     * The result is accepted by both WNDCLASSEX::hbrBackground and FillRect,
     * which the windowing/GDI subsystem maps to the corresponding system color
     * brush at use time.
     *
     * @param colorIndex One of the COLOR_* values (e.g. COLOR_BTNFACE, COLOR_WINDOW).
     */
    inline HBRUSH sys_color_brush(int colorIndex) noexcept {
        // NOLINTNEXTLINE(performance-no-int-to-ptr,cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<HBRUSH>(static_cast<INT_PTR>(colorIndex + 1));
    }

} // namespace wlx
