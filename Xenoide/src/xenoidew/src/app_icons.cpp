#include "app_icons.h"

#include "resource.h"
#include <winlambxe/utils/menu_icons.h>

namespace {

// Resource IDs for each IconId in declaration order.
constexpr int kIconResources[] = { IDI_FILE_NEW, IDI_FILE_OPEN, IDI_FILE_SAVE, IDI_FILE_SAVEALL };
constexpr size_t kIconCount = sizeof(kIconResources) / sizeof(kIconResources[0]);

constexpr int kSmallSize = 16;
constexpr int kLargeSize = 24;

} // namespace

AppIcons::~AppIcons() {
    for (HBITMAP b : _menuBitmaps) {
        if (b) DeleteObject(b);
    }
}

void AppIcons::load(HINSTANCE hInst) {
    // ImageList_Create rejects szInitial == 0 on some comctl32 versions; pad to 1.
    const WORD szInitial = static_cast<WORD>(kIconCount ? kIconCount : 1);

    _small.create({kSmallSize, kSmallSize}, ILC_COLOR32 | ILC_MASK, szInitial, 4);
    _large.create({kLargeSize, kLargeSize}, ILC_COLOR32 | ILC_MASK, szInitial, 4);

    _menuBitmaps.assign(kIconCount, nullptr);

    for (size_t i = 0; i < kIconCount; ++i) {
        const int rid = kIconResources[i];

        _small.load_from_resource(rid, hInst);
        _large.load_from_resource(rid, hInst);

        HICON ico = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(rid),
            IMAGE_ICON, kSmallSize, kSmallSize, LR_DEFAULTCOLOR));
        if (ico) {
            _menuBitmaps[i] = wlx::icon_to_alpha_bitmap(ico, kSmallSize, kSmallSize);
            DestroyIcon(ico);
        }
    }
}

HIMAGELIST AppIcons::small_image_list() const noexcept {
    return _small.himagelist();
}

HIMAGELIST AppIcons::large_image_list() const noexcept {
    return _large.himagelist();
}

int AppIcons::index_of(IconId id) const noexcept {
    if (id == IconId::none) return -1;
    const int i = static_cast<int>(id);
    return (i >= 0 && static_cast<size_t>(i) < _menuBitmaps.size()) ? i : -1;
}

HBITMAP AppIcons::menu_bitmap(IconId id) const noexcept {
    const int i = index_of(id);
    return i < 0 ? nullptr : _menuBitmaps[static_cast<size_t>(i)];
}

void AppIcons::attach_to_menu(HMENU menu, unsigned cmdId, IconId id) const noexcept {
    if (HBITMAP bmp = menu_bitmap(id)) {
        wlx::set_menu_item_bitmap(menu, cmdId, bmp);
    }
}
