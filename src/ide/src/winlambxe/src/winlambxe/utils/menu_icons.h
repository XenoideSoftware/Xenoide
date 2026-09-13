#pragma once

#include <Windows.h>

namespace wlx {

// Renders an HICON into a 32-bpp top-down DIB section. The returned HBITMAP
// is owned by the caller; release it with DeleteObject when no longer needed.
//
// Suitable for SetMenuItemInfoW with MIIM_BITMAP. On Vista+ the menu draws
// the bitmap with per-pixel alpha. On 98/2000/XP the binary mask is honored
// but no blending occurs, so transparent pixels show as the menu background.
HBITMAP icon_to_alpha_bitmap(HICON ico, int cx, int cy) noexcept;

// Sets the bitmap displayed next to a command-id menu item, or clears it
// when bitmap is nullptr. The bitmap is referenced (not copied), so it must
// outlive the menu. Returns true on success.
bool set_menu_item_bitmap(HMENU menu, unsigned cmdId, HBITMAP bitmap) noexcept;

} // namespace wlx
