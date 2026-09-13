#include "winlambxe/utils/menu_icons.h"

namespace wlx {

HBITMAP icon_to_alpha_bitmap(HICON ico, int cx, int cy) noexcept {
    if (!ico || cx <= 0 || cy <= 0) return nullptr;

    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = cx;
    bmi.bmiHeader.biHeight      = -cy;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    HDC screen = GetDC(nullptr);
    if (!screen) return nullptr;
    HDC mem = CreateCompatibleDC(screen);
    ReleaseDC(nullptr, screen);
    if (!mem) return nullptr;

    void* bits = nullptr;
    HBITMAP dib = CreateDIBSection(mem, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!dib) {
        DeleteDC(mem);
        return nullptr;
    }

    HGDIOBJ old = SelectObject(mem, dib);
    DrawIconEx(mem, 0, 0, ico, cx, cy, 0, nullptr, DI_NORMAL);
    SelectObject(mem, old);
    DeleteDC(mem);
    return dib;
}

bool set_menu_item_bitmap(HMENU menu, unsigned cmdId, HBITMAP ) noexcept {
    MENUITEMINFO mii{};
    mii.cbSize   = sizeof(mii);
    mii.fMask    = MIIM_BITMAP;
    // TODO: MENUITEMINFO::hbmpItem doesn't exists
    // mii.hbmpItem = bitmap;
    return SetMenuItemInfo(menu, cmdId, FALSE, &mii) != FALSE;
}

} // namespace wlx
