#pragma once
#include <Windows.h>

namespace wlx {

/**
 * @brief RAII wrapper for DeferWindowPos operations.
 *
 * Manages the lifecycle of an HDWP handle. The class starts a defer operation
 * with an initial number of windows upon construction. As defer operations
 * are called, the handle is naturally updated. Upon destruction, the handle
 * is submitted via EndDeferWindowPos, ensuring that layout changes occur atomically.
 */
class defer_window_pos final {
private:
    HDWP _hdwp;

public:
    /**
     * @brief Initializes a deferred window position session.
     *
     * Calls BeginDeferWindowPos with the specified initial number of windows.
     * If the allocation fails, the internal handle remains null.
     *
     * @param num_windows The initial number of windows for which position information will be stored.
     */
    explicit defer_window_pos(int num_windows = 1) noexcept;

    ~defer_window_pos();

    // Non-copyable
    defer_window_pos(const defer_window_pos&) = delete;
    defer_window_pos& operator=(const defer_window_pos&) = delete;

    // Move semantics
    defer_window_pos(defer_window_pos&& other) noexcept;

    defer_window_pos& operator=(defer_window_pos&& other) noexcept;

    /**
     * @brief Updates the deferred position for a specified window.
     *
     * Checks if the internal HDWP handle is valid and calls DeferWindowPos,
     * replacing the handle with the newly returned one if successful.
     *
     * @param hwnd     Handle to the window.
     * @param hwndInsertAfter Handle to the window to precede the positioned window in the Z order. Can be null.
     * @param x        The x-coordinate of the window's upper-left corner.
     * @param y        The y-coordinate of the window's upper-left corner.
     * @param cx       The new width of the window.
     * @param cy       The new height of the window.
     * @param flags    Window sizing and positioning flags.
     * @return true if successful or false if the handle is invalid or the operation failed.
     */
    bool defer(HWND hwnd, HWND hwndInsertAfter, int x, int y, int cx, int cy, UINT flags) noexcept;
};

} // namespace wlx
