#pragma once

#include <Windows.h>
#include <commctrl.h>       // IMAGELISTDRAWPARAMS / IMAGEINFO predeclared for commoncontrols.h
#include <shellapi.h>       // SHGetFileInfoW used inside winlamb/icon.h
#include <commoncontrols.h> // SHGetImageList used inside winlamb/icon.h
#include <vector>
#include <winlamb/image_list.h>

// Catalog of every icon glyph the app ships.
//
// Add an entry here when a new glyph resource is registered in
// app_icons.cpp's kIconResources. The integer value of each entry is the
// strip index in AppIcons::small_image_list() / large_image_list() and
// also indexes the parallel menu-bitmap table.
//
// Keep `count` last.
enum class IconId : int { none = -1, fileNew, fileOpen, fileSave, fileSaveAll, count };

// Loads and owns the application's icon glyphs in two resolutions and
// pre-renders 16x16 menu bitmaps for option-B (MIIM_BITMAP) menus.
//
// Operates correctly when no glyphs are registered yet: load() creates
// empty image lists and attach_to_menu() becomes a no-op, so callers can
// wire the API now and fill the catalog later without further changes.
class AppIcons {
public:
    AppIcons() = default;
    AppIcons(const AppIcons &) = delete;
    AppIcons &operator=(const AppIcons &) = delete;
    AppIcons(AppIcons &&) = delete;
    AppIcons &operator=(AppIcons &&) = delete;
    ~AppIcons();

    // Builds both image lists and the menu-bitmap table from the static
    // resource catalog. Call once after the main module is available.
    void load(HINSTANCE hInst);

    HIMAGELIST small_image_list() const noexcept; // 16x16, for trees / menus
    HIMAGELIST large_image_list() const noexcept; // 24x24, for toolbars

    int index_of(IconId id) const noexcept;
    HBITMAP menu_bitmap(IconId id) const noexcept;

    // Convenience for menu builders. No-op when no glyph is registered
    // for `id` (so it is safe to call before glyph art exists).
    void attach_to_menu(HMENU menu, unsigned cmdId, IconId id) const noexcept;

private:
    wl::image_list _small;
    wl::image_list _large;
    std::vector<HBITMAP> _menuBitmaps; // owned, parallel to IconId values
};
