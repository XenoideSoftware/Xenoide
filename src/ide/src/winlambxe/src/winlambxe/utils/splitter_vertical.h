#pragma once
#include <Windows.h>
#include "winlambxe/utils/splitter_base.h"
#include "winlambxe/utils/defer_window_pos.h"

namespace wlx {

/**
 * @brief Vertical splitter control dividing left and right panes.
 *
 * It occupies the entire provided background area and lays out its two
 * child edits to the left and right, preserving the bar gap at `_split_pos`.
 */
class splitter_vertical : public splitter_base<splitter_vertical> {
public:
    splitter_vertical();

    LPTSTR get_cursor() const noexcept;

    int get_primary_size(int w, int h) const noexcept;

    void on_layout(int w, int h) noexcept;

    void on_drag(int x, int y) noexcept;
};

} // namespace wlx
