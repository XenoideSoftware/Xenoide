/**
 * @file left_panel.h
 * @brief Left-side explorer panel hosting a sample treeview.
 *
 * Part of ModernWinApp.
 */

#pragma once
#include <Windows.h>
#include "winlamb/window_control.h"
#include "winlamb/treeview.h"

/**
 * @brief Resizable left panel containing a sample project treeview.
 *
 * Inherits wl::window_control so it can be embedded as a child window
 * inside the main application window and receives its own message pump.
 */
class LeftPanel : public wl::window_control {
private:
    wl::treeview _treeview;
    HIMAGELIST   _pendingImageList = nullptr; ///< Stashed until WM_CREATE wires the tree.

    static constexpr int IDC_TREEVIEW_INNER = 101;

public:
    /**
     * @brief Constructs the panel and registers WM_CREATE / WM_SIZE handlers.
     *
     * The window class name `WL_LEFTPANEL` is assigned here, before create()
     * is called by the parent.
     */
    LeftPanel();

    LeftPanel(LeftPanel&&) = default;
    LeftPanel& operator=(LeftPanel&&) = default; ///< Move-only.

    /**
     * @brief Sets the image list the inner tree will use as its TVSIL_NORMAL list.
     *
     * Safe to call before or after create(): if the tree exists, the list is
     * applied immediately; otherwise it is stashed and applied during WM_CREATE.
     * The image list is referenced (not owned) and must outlive the tree.
     */
    void set_tree_image_list(HIMAGELIST il) noexcept;
};
