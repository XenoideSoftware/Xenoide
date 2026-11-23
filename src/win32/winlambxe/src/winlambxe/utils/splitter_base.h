#pragma once
#include <Windows.h>
#include <WindowsX.h>
#include "winlamb/window_control.h"

namespace wlx {

    /**
     * @brief Specifies the anchor policy during parent resizing.
     */
    enum class splitter_resize_mode {
        anchor_first,  ///< The first pane (Left/Top) holds a fixed size.
        anchor_second, ///< The second pane (Right/Bottom) holds a fixed size.
        proportional   ///< Both panes scale proportionally.
    };

    /**
     * @brief Base CRTP class template for window splitter controls.
     *
     * Implements common splitter logic such as mouse capture, cursor handling,
     * and mouse movement tracking. The splitter window itself occupies the
     * entire client area allocated to it by its parent, and it arranges its
     * two assigned child windows over itself, leaving a gap for dragging.
     *
     * @tparam Derived The specialized splitter class.
     */
    template <typename Derived> class splitter_base : public wl::window_control {
    protected:
        HWND _hwnd1 = nullptr;     ///< Handle to the first window (Left or Top).
        HWND _hwnd2 = nullptr;     ///< Handle to the second window (Right or Bottom).
        bool _is_dragging = false; ///< True if the user is currently capturing the mouse.

        int _split_pos = -1;      ///< Current coordinate position of the splitter (-1 for auto-center).
        int _split_thickness = 6; ///< Thickness of the splitter bar in pixels.

        splitter_resize_mode _mode = splitter_resize_mode::anchor_first;
        int _prev_primary_size = 0; ///< The last recorded size to track mathematical deltas.

    public:
        /**
         * @brief Assigns the two sibling windows to be resized.
         *
         * @param hwnd1 Handle to the first window (e.g., top or left).
         * @param hwnd2 Handle to the second window (e.g., bottom or right).
         */
        void set_windows(HWND hwnd1, HWND hwnd2) noexcept {
            _hwnd1 = hwnd1;
            _hwnd2 = hwnd2;
        }

        /**
         * @brief Manually sets the position of the splitter.
         * @param pos The new coordinate coordinate.
         */
        void set_split_pos(int pos) noexcept {
            _split_pos = pos;
            if (this->hwnd()) {
                RECT rc;
                GetClientRect(this->hwnd(), &rc);
                _prev_primary_size = static_cast<Derived *>(this)->get_primary_size(rc.right, rc.bottom);
                static_cast<Derived *>(this)->on_layout(rc.right, rc.bottom);
            }
        }

        /**
         * @brief Changes the mathematical adjustment logic processing when the parent bounds change.
         * @param mode The selected enumerator mode.
         */
        void set_resize_mode(splitter_resize_mode mode) noexcept {
            _mode = mode;
        }

    protected:
        /**
         * @brief Initializes standard message handlers for drag operations.
         */
        void setup_handlers() {
            this->on_message(WM_SETCURSOR, [this](wl::params) -> LRESULT {
                SetCursor(LoadCursor(nullptr, static_cast<Derived *>(this)->get_cursor()));
                return TRUE;
            });

            this->on_message(WM_LBUTTONDOWN, [this](wl::params) -> LRESULT {
                if (_hwnd1 && _hwnd2) {
                    SetCapture(this->hwnd());
                    _is_dragging = true;
                }
                return 0;
            });

            this->on_message(WM_MOUSEMOVE, [this](wl::params p) -> LRESULT {
                if (_is_dragging) {
                    int x = GET_X_LPARAM(p.lParam);
                    int y = GET_Y_LPARAM(p.lParam);
                    static_cast<Derived *>(this)->on_drag(x, y);
                }
                return 0;
            });

            this->on_message(WM_LBUTTONUP, [this](wl::params) -> LRESULT {
                if (_is_dragging) {
                    ReleaseCapture();
                    _is_dragging = false;
                }
                return 0;
            });

            this->on_message(WM_SIZE, [this](wl::params p) -> LRESULT {
                if (p.wParam != SIZE_MINIMIZED && _hwnd1 && _hwnd2) {
                    int w = LOWORD(p.lParam);
                    int h = HIWORD(p.lParam);

                    int new_size = static_cast<Derived *>(this)->get_primary_size(w, h);
                    if (_prev_primary_size > 0 && new_size != _prev_primary_size) {
                        if (_mode == splitter_resize_mode::anchor_second) {
                            int delta = new_size - _prev_primary_size;
                            _split_pos += delta;
                        } else if (_mode == splitter_resize_mode::proportional) {
                            float ratio = static_cast<float>(_split_pos) / _prev_primary_size;
                            _split_pos = static_cast<int>(new_size * ratio);
                        }
                    }
                    _prev_primary_size = new_size;

                    static_cast<Derived *>(this)->on_layout(w, h);
                }
                return 0;
            });
        }
    };

} // namespace wlx
